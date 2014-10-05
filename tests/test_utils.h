#include <stdlib.h>

void random_string(char* str, size_t str_len)
{
  static const char bases[] = "ACTG";
  size_t i = 0;
  for(i = 0; i < str_len; i++) {
    str[i] = bases[rand() % (sizeof(bases) - 1)];
  }
}

void random_sizes(size_t* arr, size_t arr_len)
{
  static const size_t MAX_VAL = 10000;
  size_t i = 0;
  for(i = 0; i < arr_len; i++) {
    arr[i] = rand() % MAX_VAL;
  }
}
