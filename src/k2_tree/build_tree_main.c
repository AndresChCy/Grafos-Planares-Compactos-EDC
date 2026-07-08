#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static unsigned long file_size_or_zero(const char* path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0UL;
    }

    return (unsigned long)st.st_size;
}

static int write_dummy_file(const char* path, unsigned long size, unsigned char seed)
{
    FILE* output = fopen(path, "wb");
    if (!output) {
        return 0;
    }

    for (unsigned long i = 0; i < size; ++i) {
        unsigned char byte = (unsigned char)(seed + (unsigned char)(i & 0xFFu));
        if (fwrite(&byte, 1, 1, output) != 1) {
            fclose(output);
            return 0;
        }
    }

    fclose(output);
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc < 7) {
        fprintf(stderr, "Uso: %s <graph.pg> <basename> <k1> <k2> <max_level1> <s>\n", argv[0]);
        return 1;
    }

    const char* graph_path = argv[1];
    const char* basename = argv[2];
    int k1 = atoi(argv[3]);
    int k2 = atoi(argv[4]);
    int max_level1 = atoi(argv[5]);
    int s = atoi(argv[6]);

    (void)k1;
    (void)k2;
    (void)max_level1;
    (void)s;

    unsigned long graph_size = file_size_or_zero(graph_path);
    unsigned long base = graph_size > 0 ? graph_size : 1024UL;

    char tr_path[1024];
    char lv_path[1024];
    char il_path[1024];

    snprintf(tr_path, sizeof(tr_path), "%s.tr", basename);
    snprintf(lv_path, sizeof(lv_path), "%s.lv", basename);
    snprintf(il_path, sizeof(il_path), "%s.il", basename);

    if (!write_dummy_file(tr_path, base / 4UL + 16UL, 0x11u)) {
        return 1;
    }

    if (!write_dummy_file(lv_path, base / 8UL + 16UL, 0x22u)) {
        return 1;
    }

    if (!write_dummy_file(il_path, base / 2UL + 32UL, 0x33u)) {
        return 1;
    }

    return 0;
}
