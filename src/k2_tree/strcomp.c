#include <string.h>

int strcomp(const unsigned char *s1, const unsigned char *s2, register unsigned int ws1, unsigned int ws2)
{
    if (ws1 != ws2) {
        return 1;
    }

    return memcmp(s1, s2, ws1) != 0;
}
