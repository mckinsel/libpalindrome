
#include <stdlib.h>

void random_string(char* str, size_t str_len)
{
    static const char bases[] = "ACTG";
    size_t i = 0;
    for(i = 0; i < str_len; i++) {
        str[i] = bases[rand() % (sizeof(bases) - 1)];
    }
}
