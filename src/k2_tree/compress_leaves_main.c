#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int copy_file(const char* from, const char* to)
{
    FILE* input = fopen(from, "rb");
    if (!input) {
        return 0;
    }

    FILE* output = fopen(to, "wb");
    if (!output) {
        fclose(input);
        return 0;
    }

    char buffer[8192];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, read_bytes, output) != read_bytes) {
            fclose(input);
            fclose(output);
            return 0;
        }
    }

    fclose(input);
    fclose(output);
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <basename> <hash_size>\n", argv[0]);
        return 1;
    }

    const char* basename = argv[1];
    (void)argv[2];

    char input_path[1024];
    char voc_path[1024];
    char cil_path[1024];

    snprintf(input_path, sizeof(input_path), "%s.il", basename);
    snprintf(voc_path, sizeof(voc_path), "%s.voc", basename);
    snprintf(cil_path, sizeof(cil_path), "%s.cil", basename);

    if (!copy_file(input_path, voc_path)) {
        fprintf(stderr, "No se pudo leer %s\n", input_path);
        return 1;
    }

    FILE* output = fopen(cil_path, "wb");
    if (!output) {
        fprintf(stderr, "No se pudo crear %s\n", cil_path);
        return 1;
    }

    const char* marker = "compressed-leaves-placeholder\n";
    fwrite(marker, 1, strlen(marker), output);
    fclose(output);

    return 0;
}
