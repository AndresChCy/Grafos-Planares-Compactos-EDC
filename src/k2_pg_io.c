#include "k2_pg_io.h"
#include "k2_easy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* --- Parser rápido de enteros sobre un FILE*, con buffer propio ---------
 * Los datasets reales van de cientos de miles a decenas de millones de
 * aristas (hasta ~150M tokens para Planar-25M); fscanf/token a token es
 * demasiado lento para eso, así que se lee en bloques y se parsean los
 * dígitos a mano. */

#define PG_BUFSIZE (1u << 20) /* 1 MB */

typedef struct {
    FILE * f;
    unsigned char buf[PG_BUFSIZE];
    size_t len, pos;
} PgReader;

static int pg_refill(PgReader * r) {
    r->len = fread(r->buf, 1, PG_BUFSIZE, r->f);
    r->pos = 0;
    return r->len > 0;
}

static int pg_getc(PgReader * r) {
    if (r->pos >= r->len) {
        if (!pg_refill(r)) return -1;
    }
    return r->buf[r->pos++];
}

/* Lee el próximo entero no negativo, saltando espacios/saltos de línea.
 * Devuelve 0 si pudo leer un entero, -1 si se llegó a EOF antes de
 * encontrar un dígito (fin de archivo normal) o el formato es inválido. */
static int pg_read_uint(PgReader * r, uint * out) {
    int c;
    do {
        c = pg_getc(r);
    } while (c == ' ' || c == '\n' || c == '\r' || c == '\t');
    if (c < 0) return -1;
    if (c < '0' || c > '9') return -1;

    unsigned long v = 0;
    while (c >= '0' && c <= '9') {
        v = v * 10 + (unsigned long) (c - '0');
        c = pg_getc(r);
    }
    *out = (uint) v;
    return 0;
}

int k2_read_pg_file(const char * path, PgGraph * out) {
    FILE * f = fopen(path, "r");
    if (!f) return -1;

    PgReader r;
    r.f = f;
    r.len = 0;
    r.pos = 0;

    uint n, declaredM;
    if (pg_read_uint(&r, &n) != 0) { fclose(f); return -1; }
    if (pg_read_uint(&r, &declaredM) != 0) { fclose(f); return -1; }

    /* declaredM es la cantidad de aristas no dirigidas: exactamente la
     * cantidad que vamos a guardar tras deduplicar. La usamos como
     * capacidad inicial (con un piso chico por si el header viene en 0 o
     * mal escrito) y duplicamos si hiciera falta más. */
    size_t capacity = declaredM > 0 ? (size_t) declaredM : 16;
    uint * us = (uint *) malloc(sizeof(uint) * capacity);
    uint * vs = (uint *) malloc(sizeof(uint) * capacity);
    if (!us || !vs) { free(us); free(vs); fclose(f); return -1; }

    size_t count = 0;
    uint a, b;
    while (pg_read_uint(&r, &a) == 0) {
        if (pg_read_uint(&r, &b) != 0) break; /* línea incompleta al final del archivo */

        /* Cada arista no dirigida {a,b} aparece como "a b" y "b a" en
         * algún punto del archivo (no necesariamente consecutivas: van
         * agrupadas por vértice de origen, en orden contrarreloj). Nos
         * quedamos solo con la versión a<=b, así cada arista entra una
         * sola vez. */
        if (a <= b) {
            if (count >= capacity) {
                capacity *= 2;
                uint * nus = (uint *) realloc(us, sizeof(uint) * capacity);
                uint * nvs = (uint *) realloc(vs, sizeof(uint) * capacity);
                if (!nus || !nvs) { free(nus ? nus : us); free(nvs ? nvs : vs); fclose(f); return -1; }
                us = nus;
                vs = nvs;
            }
            us[count] = a;
            vs[count] = b;
            count++;
        }
    }

    fclose(f);

    out->numNodes = n;
    out->numEdges = (ulong) count;
    out->us = us;
    out->vs = vs;
    return 0;
}

void k2_free_pg_graph(PgGraph * g) {
    if (!g) return;
    free(g->us);
    free(g->vs);
    g->us = NULL;
    g->vs = NULL;
    g->numNodes = 0;
    g->numEdges = 0;
}

TREP * k2_build_from_pg_file(const char * path) {
    PgGraph g;
    if (k2_read_pg_file(path, &g) != 0) return NULL;
    TREP * trep = k2_build_undirected(g.numNodes, g.us, g.vs, g.numEdges);
    k2_free_pg_graph(&g);
    return trep;
}

double k2_disk_size_mb(const char * basename) {
    static const char * exts[] = {".tr", ".lv", ".voc", ".cil"};
    long long total = 0;
    char path[1024];
    int i;
    for (i = 0; i < 4; i++) {
        snprintf(path, sizeof(path), "%s%s", basename, exts[i]);
        struct stat st;
        if (stat(path, &st) == 0) total += (long long) st.st_size;
    }
    return (double) total / (1024.0 * 1024.0);
}

double k2_saved_size_mb(TREP * trep, const char * basename) {
    k2_save(trep, (char *) basename);
    return k2_disk_size_mb(basename);
}
