#include "k2_easy.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* positionInTH es una global definida en kTree.c (sin 'extern' en el
 * header, es la convención de todo este código legado). Se necesita
 * aquí para poder liberarla tras compressInformationLeaves y evitar
 * fugas al construir más de un árbol en el mismo proceso. Compilar
 * con -fcommon para que esta declaración se fusione con la de kTree.c
 * en vez de dar "multiple definition" al enlazar. */
unsigned int * positionInTH;
unsigned int GBs_a_ocupar = 10;

/* Variables globales usadas por kTree.c durante la construcción
 * (declaradas en kTree.h, sin 'extern' -> es la convención del código
 * legado). Se resetean antes de cada build, tal como hace buildtree.c. */

TREP * k2_build_undirected(uint numNodes, const uint * us, const uint * vs, ulong numEdges) {
    if (numNodes == 0) return NULL;

    const int _K1 = K1; /* K1=4, K2=2, definidos como macros en kTree.h */
    const int _K2 = K2;

    /* Elegimos tamSubm = menor potencia de dos ESTRICTAMENTE mayor que
     * numNodes. Con esto, siguiendo la misma fórmula que buildtree.c
     * (part = nodes/tamSubm + 1), se garantiza part == 1: todo el grafo
     * cabe en una única submatriz. (Si se usara tamSubm == numNodes con
     * numNodes potencia de dos, la fórmula original daría part=2 y
     * quedaría una submatriz vacía sin usar; por eso el '>' estricto.) */
    uint tamSubm = 1;
    uint limite = 1024 * 1024 * 1024 * GBs_a_ocupar;
    while (tamSubm <= numNodes && (tamSubm << 1) <= limite) tamSubm <<= 1; 
    /* Dos bugs de la fórmula/código original chocan con grafos chicos:
     *  1) max_level1 = ceil(log_K1(tamSubm)) - L se vuelve negativo para
     *     tamSubm pequeño, y al pasarse como uint eso es un número enorme.
     *  2) Incluso si max_level1 llega a 0 (no negativo), las funciones de
     *     consulta (compactTreeCheckLink, compactAdjacencyList, ...) hacen
     *     "trep->maxLevel1 - 1" en aritmética SIN signo -> underflow a
     *     UINT_MAX y corrompen la búsqueda (falsos negativos silenciosos).
     * tamSubm=32 es la primera potencia de dos que deja max_level1 >= 1,
     * evitando ambos casos. Grafos con muy pocos nodos igual funcionan:
     * solo se usa una submatriz 32x32 con el resto de bits en cero. */
    if (tamSubm < 32) tamSubm = 32;

    uint nodesOrig = numNodes;
    uint nodes = tamSubm;

    int max_real_level1 = (int) ceil(log((double) nodes) / log((double) _K1)) - 1;
    int max_level1 = (int) ceil(log((double) nodes) / log((double) _K1)) - L;

    long nodes2 = 1;
    int i;
    for (i = 0; i < max_real_level1 + 1; i++) nodes2 *= _K1;
    for (i = 0; i < max_level1; i++) nodes2 = (long) ceil((double) nodes2 / _K1);
    int max_level2 = (int) ceil(log((double) nodes2) / log((double) _K2)) - 1;

    TREP * trep = createTreeRep(nodesOrig, numEdges, /*part=*/1, tamSubm,
                                 max_real_level1, max_level1, max_level2, _K1, _K2);

    /* createTreeRep (código original) NO inicializa basep1/basep2/baseq1/
     * baseq2 (esos campos solo los rellena loadTreeRepresentation). Los
     * dejamos en NULL para que destroyTreeRepresentation() pueda liberar
     * este TREP de forma segura más adelante (free(NULL) es un no-op). */
    trep->basep1 = NULL;
    trep->basep2 = NULL;
    trep->baseq1 = NULL;
    trep->baseq2 = NULL;

    numberNodes = 0;
    numberLeaves = 0;
    numberTotalLeaves = 0;

    NODE * tree = createKTree(_K1, _K2, max_real_level1, max_level1, max_level2);

    ulong e;
    ulong directedCount = 0;
    for (e = 0; e < numEdges; e++) {
        uint u = us[e];
        uint v = vs[e];
        insertNode(tree, (int) u, (int) v);
        directedCount++;
        if (u != v) { /* grafo no dirigido: se inserta también el sentido opuesto */
            insertNode(tree, (int) v, (int) u);
            directedCount++;
        }
    }

    MREP * rep = createRepresentation(tree, tamSubm, directedCount);
    insertIntoTreeRep(trep, rep, 0, 0);

    /* compressInformationLeaves (codigo original) hace zeroNode-1 con
     * zeroNode unsigned; si el grafo no tiene aristas, zeroNode queda en 0
     * y esa resta desborda (underflow) y corrompe memoria. El resto del
     * código ya trata numberOfEdges==0 como caso especial (listas vacías,
     * contains siempre falso), así que simplemente nos saltamos la
     * compresión cuando no hay nada que comprimir. */
    if (directedCount > 0) {
        compressInformationLeaves(trep);
        /* compressInformationLeaves usa estado GLOBAL (hash, _memMgr,
         * positionInTH) que initialize() sobreescribe en cada llamada sin
         * liberar el anterior -> fuga si se construye más de un árbol en
         * el mismo proceso (el mismo estado global no-reentrante que ya
         * documentaste para el grid search). trep->words y compressIL ya
         * son copias propias e independientes en este punto, así que es
         * seguro liberar el hash table ahora mismo. */
        freeHashTable();
        free(positionInTH);
        positionInTH = NULL;

        /* compressInformationLeaves lee rep->leavesInf para armar el
         * vocabulario y nunca lo libera (leak confirmado con valgrind,
         * proporcional al número de hojas). Ya no se necesita para nada
         * (contains/degree solo usan bt/bn/compressIL), así que lo
         * liberamos aquí. OJO: no lo muevas a destroyRepresentation: ese
         * mismo destructor también se usa para árboles cargados con
         * k2_load, donde este campo ni siquiera se inicializa. */
        free(rep->leavesInf);
        rep->leavesInf = NULL;
    } else {
        trep->words = NULL;
        trep->zeroNode = 0;
        trep->lenWords = 0;
        free(rep->leavesInf);
        rep->leavesInf = NULL;
    }

    return trep;
}

int neighbour(TREP * trep, uint u, uint v) {
    return compactTreeCheckLink(trep, u, v) != 0;
}

uint degree(TREP * trep, uint u) {
    uint * listady = compactTreeAdjacencyList(trep, (int) u);
    return listady[0];
}

void k2_destroy(TREP * trep) {
    if (trep) destroyTreeRepresentation(trep);
}

void k2_save(TREP * trep, char * basename) {
    saveTreeRep(trep, basename);
}

TREP * k2_load(char * basename) {
    return loadTreeRepresentation(basename);
}
