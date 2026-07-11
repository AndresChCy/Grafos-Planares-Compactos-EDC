#include "k2_partition.h"
#include "k2_pg_io.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define swap_uint(x, y) { unsigned int temp_ = *(x); *(x) = *(y); *(y) = temp_; }

/* _ttlLeaves es una global definida en kTree.c (sin 'extern' en el
 * header, convención de todo este código legado) que usa
 * partialSave/closePartialFile para acumular el total de hojas. Hay que
 * resetearla antes de cada build particionado -- si no, un segundo build
 * en el mismo proceso arranca sumando sobre el total del build anterior. */
extern ulong _ttlLeaves;

int k2_build_partitioned_to_disk(uint numNodes, const uint * us, const uint * vs,
                                  ulong numEdges, unsigned S, const char * basename) {
    if (numNodes == 0) return -1;

    uint tamSubm = 1u << S;
    if (tamSubm < 32) {
        fprintf(stderr,
            "k2_build_partitioned_to_disk: S=%u da tamSubm=%u, se ajusta a 32 "
            "(S=5) para evitar el underflow sin signo del codigo legado en "
            "submatrices muy chicas\n", S, tamSubm);
        tamSubm = 32;
    }

    const int _K1 = K1, _K2 = K2; /* K1=4, K2=2, macros fijas en kTree.h */
    uint part = numNodes / tamSubm + 1;

    int max_real_level1 = (int) ceil(log((double) tamSubm) / log((double) _K1)) - 1;
    int max_level1 = (int) ceil(log((double) tamSubm) / log((double) _K1)) - L;

    long nodes2 = 1;
    int i;
    for (i = 0; i < max_real_level1 + 1; i++) nodes2 *= _K1;
    for (i = 0; i < max_level1; i++) nodes2 = (long) ceil((double) nodes2 / _K1);
    int max_level2 = (int) ceil(log((double) nodes2) / log((double) _K2)) - 1;

    fprintf(stderr,
        "k2_build_partitioned_to_disk: n=%u m=%lu tamSubm=%u (S=%u efectivo) "
        "-> %u x %u = %u submatrices\n",
        numNodes, numEdges, tamSubm, S, part, part, part * part);

    /* TREP "liviano": openPartialFile/partialSave/closePartialFile solo
     * leen estos 9 campos escalares para el header del .il -- nunca
     * tocan trep->submatrices ni los buffers MAX_INFO, así que evitamos
     * reservarlos del todo (createTreeRep los reserva siempre, ~2.4GB,
     * que acá no hacen falta para nada durante el build). */
    TREP header;
    memset(&header, 0, sizeof(header));
    header.part = part;
    header.tamSubm = tamSubm;
    header.numberOfNodes = numNodes;
    header.numberOfEdges = numEdges;
    header.repK1 = (uint) _K1;
    header.repK2 = (uint) _K2;
    header.maxRealLevel1 = (uint) max_real_level1;
    header.maxLevel1 = (uint) max_level1;
    header.maxLevel2 = (uint) (max_level2 - L); /* mismo ajuste que createTreeRep/createKTree */

    _ttlLeaves = 0;
    openPartialFile(&header, (char *) basename);

    uint fila, columna;
    for (fila = 0; fila < part; fila++) {
        for (columna = 0; columna < part; columna++) {
            numberNodes = 0;
            numberLeaves = 0;
            numberTotalLeaves = 0;

            NODE * tree = createKTree(_K1, _K2, max_real_level1, max_level1, max_level2);

            ulong edgesInBlock = 0;
            ulong e;
            for (e = 0; e < numEdges; e++) {
                uint u = us[e], v = vs[e];
                /* {u,v} no dirigida -> hasta dos bits en la matriz: (u,v) y
                 * (v,u), que pueden caer en submatrices distintas si u y v
                 * están en "bloques" de nodos diferentes. */
                if (u / tamSubm == fila && v / tamSubm == columna) {
                    insertNode(tree, (int) (u % tamSubm), (int) (v % tamSubm));
                    edgesInBlock++;
                }
                if (u != v && v / tamSubm == fila && u / tamSubm == columna) {
                    insertNode(tree, (int) (v % tamSubm), (int) (u % tamSubm));
                    edgesInBlock++;
                }
            }

            MREP * rep = createRepresentation(tree, tamSubm, edgesInBlock);
            int hadEdges = (rep->numberOfEdges != 0);
            partialSave(&header, rep, fila, columna);
            if (!hadEdges) {
                /* partialSave (codigo original) solo libera rep->leavesInf
                 * cuando numberOfEdges!=0 -- para submatrices sin aristas
                 * queda sin liberar (leak del original, mismo patrón que ya
                 * encontramos en k2_build_undirected). Se libera acá. */
                free(rep->leavesInf);
            }
            free(rep); /* partialSave ya destruyó bt/bn */
        }
    }

    closePartialFile();
    fprintf(stderr, "k2_build_partitioned_to_disk: listo (%s.tr/.lv/.il)\n", basename);
    return 0;
}

int k2_compress_partition_to_disk(const char * basename, unsigned long hashSize) {
    char * filename = (char *) malloc(strlen(basename) + 5);
    if (!filename) return -1;

    strcpy(filename, basename);
    strcat(filename, ".il");
    FILE * fvil = fopen(filename, "r");
    if (!fvil) { free(filename); return -1; }

    uint _part, _tamSubm, _numberOfNodes, _repK1, _repK2, _maxRealLevel1, _maxLevel1, _maxLevel2;
    ulong _numberOfEdges;
    unsigned long addrInTH;
    unsigned int zeroNode = 0;
    unsigned int * positionInTH = NULL;

    if (fread(&_part, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_tamSubm, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_numberOfNodes, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_numberOfEdges, sizeof(ulong), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_repK1, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_repK2, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_maxRealLevel1, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_maxLevel1, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }
    if (fread(&_maxLevel2, sizeof(uint), 1, fvil) != 1) { fclose(fvil); free(filename); return -1; }

    uint ** _partnumberOfNodes, ** _partcutBt, ** _partnleaves, ** _partlastBt1_len;
    uint *** _partleavesInf;
    ulong ** _partnumberOfEdges;
    FTRep *** _partcompressIL;

    _partnumberOfNodes = (uint **) malloc(sizeof(uint *) * _part);
    _partcutBt = (uint **) malloc(sizeof(uint *) * _part);
    _partnleaves = (uint **) malloc(sizeof(uint *) * _part);
    _partlastBt1_len = (uint **) malloc(sizeof(uint *) * _part);
    _partleavesInf = (uint ***) malloc(sizeof(uint **) * _part);
    _partnumberOfEdges = (ulong **) malloc(sizeof(ulong *) * _part);
    _partcompressIL = (FTRep ***) malloc(sizeof(FTRep **) * _part);

    uint fila, columna, i2;
    for (i2 = 0; i2 < _part; i2++) {
        _partnumberOfNodes[i2] = (uint *) malloc(sizeof(uint) * _part);
        _partcutBt[i2] = (uint *) malloc(sizeof(uint) * _part);
        _partnleaves[i2] = (uint *) malloc(sizeof(uint) * _part);
        _partlastBt1_len[i2] = (uint *) malloc(sizeof(uint) * _part);
        _partleavesInf[i2] = (uint **) malloc(sizeof(uint *) * _part);
        _partnumberOfEdges[i2] = (ulong *) malloc(sizeof(ulong) * _part);
        _partcompressIL[i2] = (FTRep **) malloc(sizeof(FTRep *) * _part);
    }

    for (fila = 0; fila < _part; fila++) {
        for (columna = 0; columna < _part; columna++) {
            fread(&(_partnumberOfNodes[fila][columna]), sizeof(uint), 1, fvil);
            fread(&(_partnumberOfEdges[fila][columna]), sizeof(ulong), 1, fvil);
            if (_partnumberOfEdges[fila][columna] == 0) continue;
            fread(&(_partcutBt[fila][columna]), sizeof(uint), 1, fvil);
            fread(&(_partlastBt1_len[fila][columna]), sizeof(uint), 1, fvil);
            fread(&(_partnleaves[fila][columna]), sizeof(uint), 1, fvil);
            _partleavesInf[fila][columna] =
                (uint *) malloc(sizeof(uint) * _partnleaves[fila][columna] * (K2_2 * K2_2 + W - 1) / W);
            fread(_partleavesInf[fila][columna], sizeof(uint),
                  (_partnleaves[fila][columna] * K2_2 * K2_2 + W - 1) / W, fvil);
        }
    }

    ulong _totalLeaves;
    fread(&_totalLeaves, sizeof(ulong), 1, fvil);
    fclose(fvil);

    if (hashSize == 0) hashSize = _totalLeaves > 0 ? _totalLeaves : 1;

    unsigned char * ilchar = (unsigned char *) malloc((size_t) _totalLeaves * K2_3_char);
    unsigned char * aWord;
    unsigned int size;
    uint j;

    unsigned char * ilchar2 = ilchar;
    for (fila = 0; fila < _part; fila++) {
        for (columna = 0; columna < _part; columna++) {
            if (_partnumberOfEdges[fila][columna] == 0) continue;
            ulong i;
            for (i = 0; i < _partnleaves[fila][columna]; i++) {
                aWord = ilchar2;
                for (j = 0; j < K2_3; j++) {
                    if (bitget(_partleavesInf[fila][columna], i * K2_3 + j)) bitsetchar(aWord, j);
                    else bitcleanchar(aWord, j);
                }
                ilchar2 += K2_3_char;
            }
        }
    }

    for (fila = 0; fila < _part; fila++)
        for (columna = 0; columna < _part; columna++)
            free(_partleavesInf[fila][columna]);

    uint lenWords = K2_3_char;
    zeroNode = 0;
    _memMgr = createMemoryManager();
    initialize_hash(hashSize);
    positionInTH = (unsigned int *) malloc(_totalLeaves * sizeof(unsigned int));

    ulong ilpos = 0, i3;
    for (i3 = 0; i3 < _totalLeaves; i3++) {
        aWord = &(ilchar[ilpos]);
        size = K2_3_char;
        j = search((unsigned char *) aWord, size, &addrInTH);
        if (j == zeroNode) {
            insertElement((unsigned char *) aWord, size, &addrInTH);
            hash[addrInTH].weight = 0;
            hash[addrInTH].size = 0;
            hash[addrInTH].len = K2_3_char;
            positionInTH[zeroNode] = (unsigned int) addrInTH;
            zeroNode++;
        }
        hash[addrInTH].weight += 1;
        ilpos += K2_3_char;
    }

    /* Ordena el vocabulario por frecuencia (igual que compressleaves.c /
     * compressInformationLeaves original): las palabras con frecuencia 1
     * van al final, y el resto se ordena descendente por frecuencia. */
    int k = 0;
    {
        int ii = 0, kk = (int) zeroNode - 1;
        while (ii < kk) {
            while ((hash[positionInTH[ii]].weight != 1) && (ii < kk)) ii++;
            while ((hash[positionInTH[kk]].weight == 1) && (ii < kk)) kk--;
            if (ii < kk) {
                swap_uint(&positionInTH[ii], &positionInTH[kk]);
                kk--;
                ii++;
            }
        }
        k = ii + 1;
    }
    if (zeroNode > 0) qsort(positionInTH, (size_t) k, sizeof(unsigned int), comparaFrecListaDesc);

    for (i3 = 0; i3 < zeroNode; i3++) hash[positionInTH[i3]].codeword = i3;

    ilpos = 0;
    for (fila = 0; fila < _part; fila++) {
        for (columna = 0; columna < _part; columna++) {
            if (_partnumberOfEdges[fila][columna] == 0) continue;
            uint * listIL = (uint *) malloc(sizeof(uint) * _partnleaves[fila][columna]);
            uint listILCount = 0;
            ulong i;
            for (i = 0; i < _partnleaves[fila][columna]; i++) {
                aWord = &(ilchar[ilpos]);
                size = K2_3_char;
                j = search((unsigned char *) aWord, size, &addrInTH);
                listIL[listILCount++] = hash[addrInTH].codeword;
                ilpos += K2_3_char;
            }
            _partcompressIL[fila][columna] = createFT(listIL, _partnleaves[fila][columna]);
            free(listIL);
        }
    }

    unsigned char * words = (unsigned char *) malloc((size_t) zeroNode * lenWords);
    {
        uint wc = 0, ii;
        for (ii = 0; ii < zeroNode; ii++)
            for (j = 0; j < lenWords; j++)
                words[wc++] = hash[positionInTH[ii]].word[j];
    }
    free(ilchar);

    /* --- basename.voc --- */
    strcpy(filename, basename);
    strcat(filename, ".voc");
    FILE * fv = fopen(filename, "w");
    fwrite(&_part, sizeof(uint), 1, fv);
    fwrite(&_tamSubm, sizeof(uint), 1, fv);
    fwrite(&_numberOfNodes, sizeof(uint), 1, fv);
    fwrite(&_numberOfEdges, sizeof(ulong), 1, fv);
    fwrite(&_repK1, sizeof(uint), 1, fv);
    fwrite(&_repK2, sizeof(uint), 1, fv);
    fwrite(&_maxRealLevel1, sizeof(uint), 1, fv);
    fwrite(&_maxLevel1, sizeof(uint), 1, fv);
    fwrite(&_maxLevel2, sizeof(uint), 1, fv);
    fwrite(&zeroNode, sizeof(uint), 1, fv);
    fwrite(&lenWords, sizeof(uint), 1, fv);
    fwrite(words, sizeof(unsigned char), (size_t) zeroNode * lenWords, fv);
    fclose(fv);
    free(words);

    /* --- basename.cil --- */
    strcpy(filename, basename);
    strcat(filename, ".cil");
    FILE * fi = fopen(filename, "w");
    for (fila = 0; fila < _part; fila++) {
        for (columna = 0; columna < _part; columna++) {
            fwrite(&(_partnumberOfNodes[fila][columna]), sizeof(uint), 1, fi);
            fwrite(&(_partnumberOfEdges[fila][columna]), sizeof(ulong), 1, fi);
            if (_partnumberOfEdges[fila][columna] == 0) continue;
            fwrite(&(_partcutBt[fila][columna]), sizeof(uint), 1, fi);
            fwrite(&(_partlastBt1_len[fila][columna]), sizeof(uint), 1, fi);
            fwrite(&(_partnleaves[fila][columna]), sizeof(uint), 1, fi);
            saveFT(_partcompressIL[fila][columna], fi);
            destroyFT(_partcompressIL[fila][columna]);
        }
    }
    fclose(fi);

    for (i2 = 0; i2 < _part; i2++) {
        free(_partnumberOfNodes[i2]);
        free(_partcutBt[i2]);
        free(_partnleaves[i2]);
        free(_partlastBt1_len[i2]);
        free(_partleavesInf[i2]);
        free(_partnumberOfEdges[i2]);
        free(_partcompressIL[i2]);
    }
    free(_partnumberOfNodes);
    free(_partcutBt);
    free(_partnleaves);
    free(_partlastBt1_len);
    free(_partleavesInf);
    free(_partnumberOfEdges);
    free(_partcompressIL);
    free(filename);

    /* Mismo motivo que en k2_build_undirected: liberar el hash table
     * global ahora que ya no hace falta, para poder encadenar builds sin
     * fugas. positionInTH acá es LOCAL (a diferencia de
     * k2_build_undirected), así que un free simple alcanza. */
    freeHashTable();
    free(positionInTH);

    fprintf(stderr, "k2_compress_partition_to_disk: listo (%s.voc/.cil)\n", basename);
    return 0;
}

int k2_build_partitioned(uint numNodes, const uint * us, const uint * vs,
                          ulong numEdges, unsigned S, const char * basename) {
    if (k2_build_partitioned_to_disk(numNodes, us, vs, numEdges, S, basename) != 0) return -1;
    return k2_compress_partition_to_disk(basename, 0);
}

int k2_build_pg_file_partitioned(const char * path, unsigned S, const char * basename) {
    PgGraph g;
    if (k2_read_pg_file(path, &g) != 0) return -1;
    int rc = k2_build_partitioned(g.numNodes, g.us, g.vs, g.numEdges, S, basename);
    k2_free_pg_graph(&g);
    return rc;
}
