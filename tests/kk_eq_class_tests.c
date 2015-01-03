#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"
#include "kolpakov_kucherov/equivalence_class.h"
#include "kolpakov_kucherov/equivalence_class_private.h"

/* Test assignment of equivalence classes for BANANA. See the function comment
 * for create_equiv_class_tables for more detail, but the expected values are
 *
 *  forward:              reverse:
 *    0   :   1             0   :   0
 *    1   :   2             1   :   0
 *    2   :   3             2   :   0
 *    3   :   2             3   :   7
 *    4   :   0             4   :   2
 *    5   :   0             5   :   3
 *    6   :   0             6   :   2
 * 
 * Though particular values may change, it's equality that matters.
 */
char* test_banana()
{
  char str[] = "BANANA";
  size_t  str_len = sizeof(str) - 1;
  size_t substr_len = 3;

  AugmentedString_T aug_string = AugmentedString_create(str, str_len);
  EquivClassTable_T eq_table = EquivClassTable_create(aug_string, substr_len);
  
  mu_assert(EquivClassTable_forward_lookup(eq_table, 1) ==
            EquivClassTable_forward_lookup(eq_table, 3),
            "Failed to assign same eq class id to all the ANA substrings.");
  mu_assert(EquivClassTable_forward_lookup(eq_table, 3) ==
              EquivClassTable_reverse_lookup(eq_table, 4),
            "Failed to assign same eq class id to all the ANA substrings.");
  mu_assert(EquivClassTable_reverse_lookup(eq_table, 4) ==
              EquivClassTable_reverse_lookup(eq_table, 6),
            "Failed to assign same eq class id to all the ANA substrings.");

  mu_assert(EquivClassTable_forward_lookup(eq_table, 2) ==
            EquivClassTable_reverse_lookup(eq_table, 5),
            "Failed to assign same eq class id to all the NAN substrings.");
  
  mu_assert(EquivClassTable_forward_lookup(eq_table, 4) == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(EquivClassTable_forward_lookup(eq_table, 5) == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(EquivClassTable_forward_lookup(eq_table, 6) == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(EquivClassTable_reverse_lookup(eq_table, 0) == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(EquivClassTable_reverse_lookup(eq_table, 1) == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(EquivClassTable_reverse_lookup(eq_table, 2) == 0,
            "Failed to assign 0 to invalid substrings.");
  
  int rc = EquivClassTable_verify(str, str_len, eq_table, substr_len);

  mu_assert(rc == 0, "Failed equivalence class verification.");

  EquivClassTable_delete(&eq_table);
  AugmentedString_delete(&aug_string);

  return NULL;
}

char* test_eq_class_verification()
{
  char str[] = "BANANA";
  size_t  str_len = sizeof(str) - 1;
  size_t substr_len = 3;

  AugmentedString_T aug_string = AugmentedString_create(str, str_len);
  EquivClassTable_T eq_table = EquivClassTable_create(aug_string, substr_len);

  EquivClassIndex_T* good_forward = malloc(sizeof(EquivClassIndex_T)*(str_len+1));
  EquivClassIndex_T* good_reverse = malloc(sizeof(EquivClassIndex_T)*(str_len+1));
  
  memcpy(good_forward, eq_table->forward_classes, sizeof(EquivClassIndex_T)*(str_len+1));
  memcpy(good_reverse, eq_table->reverse_classes, sizeof(EquivClassIndex_T)*(str_len+1));

  eq_table->forward_classes[1] = 99;
  fprintf(stderr, "Expect eq_class warning:\n");
  int ret = EquivClassTable_verify(str, str_len, eq_table, substr_len);
  mu_assert(ret == 1, "equivalence class marked incorrect classes as correct.");

  memcpy(eq_table->forward_classes, good_forward, sizeof(EquivClassIndex_T)*(str_len+1));
  
  eq_table->reverse_classes[2] = 1;
  fprintf(stderr, "Expect eq_class warning:\n");
  ret = EquivClassTable_verify(str, str_len, eq_table, substr_len);
  mu_assert(ret == 1, "equivalence class marked incorrect classes as correct.");
  
  EquivClassTable_delete(&eq_table);
  free(good_forward);
  free(good_reverse);
  AugmentedString_delete(&aug_string);

  return NULL;
}
/* Test random strings using the verification function */
char* test_random_strings()
{
  const size_t str_len = 5000;
  char* str = calloc((str_len + 1), sizeof(char));
  size_t substr_len;

  AugmentedString_T aug_string = NULL;
  EquivClassTable_T eq_table = NULL;

  int ret = 0;
  unsigned int i = 0;
  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    substr_len = rand() % 100;

    aug_string = AugmentedString_create(str, str_len);
    eq_table = EquivClassTable_create(aug_string, substr_len);

    ret = EquivClassTable_verify(str, str_len, eq_table, substr_len);

    mu_assert(ret == 0, "Failed equivalence class verification.");

    EquivClassTable_delete(&eq_table);
    AugmentedString_delete(&aug_string);
  }
  
  free(str);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_banana);
  mu_run_test(test_random_strings);
  mu_run_test(test_eq_class_verification);

  return NULL;
}

RUN_TESTS(all_tests);
