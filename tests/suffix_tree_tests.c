#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"

char* test_random_strings()
{
    const size_t str_len = 1000;
    char* str = malloc(str_len * sizeof(char));
    unsigned int i = 0;

    for(i = 0; i < 5; i++) {
        random_string(str, str_len);
        SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
        DBL_WORD rc = ST_SelfTest(stree);

        mu_assert(rc == 1, "Suffix tree failed self test.");

        ST_DeleteTree(stree);
    }

    free(str);

    return NULL;
}

char* test_print_stree()
{
    char str[] = "BANANA";
    size_t str_len = sizeof(str) - 1;

    SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
    printf("\nSuffix tree for BANANA:");
    ST_PrintTree(stree);
    
    ST_DeleteTree(stree);    

    return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_random_strings);
  mu_run_test(test_print_stree);

  return NULL;
}

RUN_TESTS(all_tests);
