#include "dbg.h"
#include "kolpakov_kucherov/utils.h"

/*
 * Create a new string that is the input string, a separator, the reverse of
 * the input string, and a null terminator
 */
char* append_reverse(const char* query_string, size_t query_length)
{
  char* query_and_reverse = malloc(QPR_LENGTH(query_length) * sizeof(char));
  check_mem(query_and_reverse);

  strncpy(query_and_reverse, query_string, query_length);

  query_and_reverse[query_length] = '#';

  size_t i = 0;
  for(i = 0; i < query_length; i++) {
    query_and_reverse[query_length + 1 + i] = query_string[query_length - 1 - i];
  }
  query_and_reverse[QPR_LENGTH(query_length) - 1] = '\0';

  return query_and_reverse;

error:
  if(query_and_reverse) free(query_and_reverse);
  return NULL;
}

