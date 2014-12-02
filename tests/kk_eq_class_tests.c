#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"
#include "kolpakov_kucherov/equivalence_classes.h"

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
  size_t* forward_table = NULL;
  size_t* reverse_table = NULL;
  SUFFIX_TREE* stree = NULL;

  char str[] = "BANANA";
  size_t  str_len = sizeof(str) - 1;
  size_t substr_len = 3;

  create_equiv_class_tables(str, str_len, substr_len, &forward_table,
                            &reverse_table, &stree);
  
  mu_assert(forward_table[1] == forward_table[3],
            "Failed to assign same eq class id to all the ANA substrings.");
  mu_assert(forward_table[3] == reverse_table[4],
            "Failed to assign same eq class id to all the ANA substrings.");
  mu_assert(reverse_table[4] == reverse_table[6],
            "Failed to assign same eq class id to all the ANA substrings.");

  mu_assert(forward_table[2] == reverse_table[5],
            "Failed to assign same eq class id to all the NAN substrings.");
  
  mu_assert(forward_table[4] == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(forward_table[5] == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(forward_table[6] == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(reverse_table[0] == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(reverse_table[1] == 0,
            "Failed to assign 0 to invalid substrings.");
  mu_assert(reverse_table[2] == 0,
            "Failed to assign 0 to invalid substrings.");
  
  int rc = verify_equiv_class_tables(str, str_len, substr_len,
                                     forward_table, reverse_table);

  mu_assert(rc == 0, "Failed equivalence class verification.");

  free(forward_table);
  free(reverse_table);
  ST_DeleteTree(stree);

  return NULL;
}

char* test_eq_class_verification()
{
  size_t* forward_table = NULL;
  size_t* reverse_table = NULL;
  SUFFIX_TREE* stree = NULL;

  char str[] = "BANANA";
  size_t  str_len = sizeof(str) - 1;
  size_t substr_len = 3;

  create_equiv_class_tables(str, str_len, substr_len, &forward_table,
                            &reverse_table, &stree);
  size_t* bad_forward = malloc(sizeof(size_t)*(str_len+1));
  size_t* bad_reverse = malloc(sizeof(size_t)*(str_len+1));

  memcpy(bad_forward, forward_table, sizeof(size_t)*(str_len+1));
  memcpy(bad_reverse, reverse_table, sizeof(size_t)*(str_len+1));
  bad_forward[1] = 99;
  fprintf(stderr, "Expect eq_class warning:\n");
  int ret = verify_equiv_class_tables(str, str_len, substr_len, bad_forward, bad_reverse);
  mu_assert(ret == 1, "equivalence class marked incorrect classes as correct.");

  memcpy(bad_forward, forward_table, sizeof(size_t)*(str_len+1));
  memcpy(bad_reverse, reverse_table, sizeof(size_t)*(str_len+1));
  bad_reverse[2] = 1;
  fprintf(stderr, "Expect eq_class warning:\n");
  ret = verify_equiv_class_tables(str, str_len, substr_len, bad_forward, bad_reverse);
  mu_assert(ret == 1, "equivalence class marked incorrect classes as correct.");
  
  free(forward_table);
  free(reverse_table);
  free(bad_forward);
  free(bad_reverse);
  ST_DeleteTree(stree); 

  return NULL;
}

/* Test random strings using the verification function */
char* test_random_strings()
{
  const size_t str_len = 5000;
  char* str = calloc((str_len + 1), sizeof(char));
  size_t substr_len;

  size_t* forward_table = NULL;
  size_t* reverse_table = NULL;
  SUFFIX_TREE* stree = NULL;

  int ret = 0;
  unsigned int i = 0;
  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    substr_len = rand() % 100;
    create_equiv_class_tables(str, str_len, substr_len, &forward_table,
                           &reverse_table, &stree);

    ret = verify_equiv_class_tables(str, str_len, substr_len,
                                 forward_table, reverse_table);

    mu_assert(ret == 0, "Failed equivalence class verification.");

    free(forward_table);
    free(reverse_table);
    ST_DeleteTree(stree);
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
