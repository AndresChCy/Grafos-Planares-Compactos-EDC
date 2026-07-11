#ifndef K2_PARTITION_H
#define K2_PARTITION_H

/*
 * Build PARTICIONADO del k2-tree: la alternativa a k2_build_undirected
 * (k2_easy.h) para grafos que NO entran cómodos en RAM como una sola
 * submatriz.
 *
 * Es un port de buildtree.c + compressleaves.c (los dos binarios CLI
 * originales que ya usan este mismo enfoque) a funciones de biblioteca,
 * pero leyendo las aristas desde un array en memoria (ya parseado, p.ej.
 * con k2_pg_io.h) en vez de releer el archivo de disco una vez por cada
 * submatriz como hace el buildtree.c original.
 *
 * Idea: en vez de construir el árbol COMPLETO del grafo en memoria de
 * una (lo que hace k2_build_undirected), se divide la matriz de
 * adyacencia n×n en submatrices de tamSubm×tamSubm (tamSubm = 1<<S) y se
 * construye, guarda a disco y libera UNA submatriz a la vez. En ningún
 * momento hay más de una submatriz completa en RAM simultáneamente (más
 * los ~2.4GB fijos de MAX_INFO que trae createTreeRep... salvo que ACÁ no
 * se llama a createTreeRep en absoluto durante el build: openPartialFile/
 * partialSave/closePartialFile solo necesitan 9 campos escalares del
 * header, nunca tocan trep->submatrices ni los buffers MAX_INFO, así que
 * se arma un TREP "liviano" con esos 9 campos nada más y así también nos
 * ahorramos esos 2.4GB durante el build particionado).
 *
 * S tiene el mismo piso de 5 (tamSubm=32) que k2_build_undirected, por el
 * mismo bug de underflow sin signo del código legado en grafos/
 * submatrices muy chicas.
 *
 * Trade-off (ver README): S más chico -> más submatrices -> más lento
 * (se escanea el array de aristas una vez por cada una) pero menos RAM
 * pico. S más grande -> menos submatrices -> más rápido pero más RAM
 * pico por submatriz. Con S grande al punto de que part==1 esto es
 * equivalente a k2_build_undirected (pero más lento por el paso extra a
 * disco), así que para grafos que ya entran en RAM como una submatriz
 * conviene seguir usando k2_build_undirected directo.
 */

#include "kTree.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fase 1: construye cada submatriz y la escribe a basename.tr/.lv/.il,
 * liberando cada una antes de pasar a la siguiente. NO deja nada
 * usable para consultas todavía (falta comprimir). 0 = éxito, -1 = error. */
int k2_build_partitioned_to_disk(uint numNodes, const uint * us, const uint * vs,
                                  ulong numEdges, unsigned S, const char * basename);

/* Fase 2: lee basename.il (todas las hojas sin comprimir, acumuladas por
 * la fase 1) y escribe basename.voc/basename.cil ya comprimidos con DAC.
 * hashSize es el tamaño del hash table para el vocabulario de patrones
 * de hoja -- 0 = automático (usa el total de hojas leído de basename.il,
 * igual que la versión in-memory de k2_build_undirected). Un valor más
 * chico ahorra RAM en ESTA fase a costa de más colisiones en el hash
 * (sigue siendo correcto, solo más lento). 0 = éxito, -1 = error. */
int k2_compress_partition_to_disk(const char * basename, unsigned long hashSize);

/* Atajo: fase 1 + fase 2. Tras esto, usá k2_load(basename) (de
 * k2_easy.h) para obtener un TREP* y consultar con neighbour/degree
 * normalmente -- desde ese punto es exactamente el mismo tipo de objeto
 * que devuelve k2_build_undirected. */
int k2_build_partitioned(uint numNodes, const uint * us, const uint * vs,
                          ulong numEdges, unsigned S, const char * basename);

/* Atajo desde archivo .pg directo (lee con k2_pg_io.h + build_partitioned).*/
int k2_build_pg_file_partitioned(const char * path, unsigned S, const char * basename);

#ifdef __cplusplus
}
#endif

#endif
