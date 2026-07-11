#ifndef K2_PG_IO_H
#define K2_PG_IO_H

/*
 * Loader para el formato de datasets de grafos planares de
 * https://www.inf.udec.cl/~jfuentess/datasets/graphs.php
 *
 * Formato (texto plano, separado por espacios/saltos de línea):
 *
 *   <numero de vertices>
 *   <numero de aristas>
 *   <origen> <destino>
 *   <origen> <destino>
 *   ...
 *
 * - "numero de aristas" es la cantidad de aristas NO DIRIGIDAS.
 * - Cada arista no dirigida aparece DOS veces en el archivo (una vez por
 *   cada extremo, en orden contrarreloj alrededor de cada vértice) -> el
 *   archivo tiene el doble de líneas "origen destino" que "numero de
 *   aristas" declarado en el header. Si asumís que la cantidad de líneas
 *   es igual al header (en vez de leer hasta EOF) terminás leyendo solo
 *   la mitad del grafo real.
 *
 * k2_read_pg_file ya deduplica ambos sentidos (se queda con la línea
 * donde origen <= destino) y devuelve cada arista UNA sola vez, lista
 * para pasar directo a k2_build_undirected (que inserta ambos sentidos
 * por su cuenta).
 */

#include "kTree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint numNodes;
    ulong numEdges; /* aristas no dirigidas, ya deduplicadas */
    uint * us;
    uint * vs;
} PgGraph;

/* Lee un archivo .pg completo a memoria. Devuelve 0 en éxito, -1 si no se
 * pudo abrir o el archivo no tiene ni siquiera el header válido. */
int k2_read_pg_file(const char * path, PgGraph * out);

/* Libera lo reservado por k2_read_pg_file. */
void k2_free_pg_graph(PgGraph * g);

/* Atajo: lee el archivo Y construye el TREP en un solo paso, liberando
 * la lista de aristas intermedia antes de devolver el árbol. NULL si el
 * archivo no se pudo leer. */
TREP * k2_build_from_pg_file(const char * path);

/* Tamaño en disco de la representación ya construida: usa k2_save (mismo
 * formato que saveTreeRep original) y suma el tamaño real en bytes de los
 * 4 archivos resultantes (.tr/.lv/.voc/.cil). Es el mismo criterio que ya
 * usás en K2TreeBuilder::compressionSizes(): tamaño real en disco, no una
 * estimación in-memory. basename se usa como prefijo temporal (podés
 * apuntar a /tmp). Devuelve el tamaño en MB (bytes/1024/1024). */
double k2_saved_size_mb(TREP * trep, const char * basename);

/* Igual que k2_saved_size_mb pero SIN guardar primero -- para cuando ya
 * construiste con k2_build_partitioned (que ya deja basename.tr/.lv/.voc/
 * .cil escritos) y no querés volver a guardarlos. */
double k2_disk_size_mb(const char * basename);

#ifdef __cplusplus
}
#endif

#endif
