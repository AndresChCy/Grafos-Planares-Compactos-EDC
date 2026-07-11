#ifndef K2_EASY_H
#define K2_EASY_H

/*
 * k2_easy: capa delgada sobre el kTree.c/h legado original.
 *
 * Sigue EXACTAMENTE el mismo pipeline que buildtree.c + compressleaves.c
 * (createTreeRep -> createKTree -> insertNode* -> createRepresentation ->
 *  insertIntoTreeRep -> compressInformationLeaves), pero:
 *
 *   1) Todo en un solo proceso, en memoria (sin pasar por los archivos
 *      intermedios .il/.tr/.lv/.voc/.cil que usan buildtree/compressleaves).
 *   2) Asume part = 1 (una sola submatriz). Esto es correcto siempre que el
 *      grafo completo quepa en una submatriz tamSubm x tamSubm, lo cual
 *      k2_build_undirected garantiza eligiendo tamSubm automáticamente.
 *   3. Pensado para grafos NO DIRIGIDOS: cada arista {u,v} se inserta en
 *      ambas direcciones (u,v) y (v,u), tal como requiere kTree.c para
 *      que compactTreeCheckLink/compactTreeAdjacencyList vean el grafo
 *      como simétrico.
 *
 * IMPORTANTE (memoria):
 * createTreeRep (código original, sin modificar) reserva varios buffers
 * de tamaño fijo MAX_INFO = 1024*1024*100+10 uints (~400MB cada uno,
 * son 6 buffers -> ~2.4GB) sin importar el tamaño real del grafo. Es la
 * misma limitación que ya documentaste y resolviste en K2TreeBuilder.
 * Aquí se deja el código legado tal cual (tal como pediste), así que si
 * vas a llamar a k2_build_undirected muchas veces en un loop, hazlo en
 * subprocesos separados (como ya haces en el grid search), no en el mismo
 * proceso repetidamente.
 */

#include "kTree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Construye la representación k2-tree de un grafo NO DIRIGIDO.
 *
 *   numNodes   : cantidad de vértices (0..numNodes-1)
 *   us, vs     : arrays paralelos con los extremos de cada arista.
 *                Cada arista se da UNA sola vez (no dupliques u<->v,
 *                la función ya inserta ambos sentidos).
 *   numEdges   : cantidad de aristas (longitud de us/vs)
 *
 * Devuelve un TREP* listo para usarse con k2_neighbour / degree.
 * NULL si numNodes == 0.
 */
TREP * k2_build_undirected(uint numNodes, const uint * us, const uint * vs, ulong numEdges);

/* neighbour(u, v): true si existe la arista {u,v}. O(profundidad del árbol). */
int neighbour(TREP * trep, uint u, uint v);

/* degree(u): grado de u. Para grafos no dirigidos, in-degree == out-degree,
 * así que basta con la lista de adyacencia saliente. */
uint degree(TREP * trep, uint u);

/* Libera todo lo reservado por k2_build_undirected (o por k2_load). */
void k2_destroy(TREP * trep);

/* --- Persistencia, reutilizando el pipeline original de guardado --- */

/* Guarda en disco con el mismo formato que saveTreeRep (basename.tr/.lv/.voc/.cil) */
void k2_save(TREP * trep, char * basename);

/* Carga con el loadTreeRepresentation original. El TREP resultante es
 * intercambiable con el que devuelve k2_build_undirected: mismos
 * k2_neighbour/degree/k2_destroy sirven para ambos. */
TREP * k2_load(char * basename);

#ifdef __cplusplus
}
#endif

#endif
