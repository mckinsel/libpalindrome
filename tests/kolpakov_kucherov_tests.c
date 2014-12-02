#include "minunit.h"
#include "suffix_tree/suffix_tree.h"
#include "kolpakov_kucherov/utils.h"

char* test_create_query_and_reverse()
{
  char str[] = "ABCDEFG";
  size_t str_len = sizeof(str) - 1;

  char* with_reverse = append_reverse(str, str_len);

  mu_assert(strcmp(with_reverse, "ABCDEFG#GFEDCBA") == 0,
            "Failed to create string with reverse."); 
  
  free(with_reverse);
  return NULL;
}



char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_create_query_and_reverse);

  return NULL;
}

RUN_TESTS(all_tests);
