#include "format_detector.h"
#include "common.h"
#include <stdio.h>

int detect_format(const char *filename, int *is_ogg) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    unsigned char magic[8];
    if (fread(magic, 1, 8, f) != 8) {
        fclose(f);
        return 0;
    }

    if (memcmp(magic, "OpusHead", 8) == 0) {
        *is_ogg = 0;
        fclose(f);
        return 1;
    } else if (memcmp(magic, "OggS", 4) == 0) {
        *is_ogg = 1;
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}
