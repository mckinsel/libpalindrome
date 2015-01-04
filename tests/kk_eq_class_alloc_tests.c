#include "failing_allocs.h"
#include "minunit.h"

#include "kolpakov_kucherov/equivalence_class.h"

USE_FAILING_ALLOCS

char* test_eq_table_allocs()
{
  char str[] = "BANANA";
  size_t  str_len = sizeof(str) - 1;
  size_t substr_len = 3;
  
  AugmentedString_T aug_string = NULL; 
  EquivClassTable_T eq_table = NULL;
  int i = 0;

  for(i = 0; i < 25; i++) {

    aug_string = NULL;
    eq_table = NULL;

    aug_string = AugmentedString_create(str, str_len);

    if(aug_string) {
      eq_table = EquivClassTable_create(aug_string, substr_len);
      
      if(eq_table) {
        int rc = EquivClassTable_verify(str, str_len, eq_table, substr_len);
        mu_assert(rc == 0, "Failed equivalence class table verification.");
      }
    }
    EquivClassTable_delete(&eq_table);
    AugmentedString_delete(&aug_string);
  }

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_eq_table_allocs);
  
  FREE_FAILING_ALLOCS

  return NULL;
}

RUN_TESTS(all_tests);
