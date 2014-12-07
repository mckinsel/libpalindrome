#include "minunit.h"
#include "test_utils.h"
#include "kolpakov_kucherov/kolpakov_kucherov.h"


char* test_madam_im_adam()
{

  char str[] = "MADAMIBCDEMADAM";
  size_t str_len = sizeof(str)/sizeof(char);
  
  length_constrained_palindromes(str, str_len, 3, 2, 5);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_madam_im_adam);
  return NULL;
}

RUN_TESTS(all_tests);
