#include "failing_allocs.h"
#include "minunit.h"
#include "test_utils.h"

#include "lca/lca_suffix_tree.h"

USE_FAILING_ALLOCS

char* test_lca_suffix_tree_allocs()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  LCASuffixTree_T tree = NULL; 
  int i = 0;
  int ret_val = 1;
  for(i = 0; i < 250; i++){
    tree = LCASuffixTree_create(str, str_len);
    if(tree) {
      ret_val = LCASuffixTree_verify(tree);
    }
    LCASuffixTree_delete(&tree);
  }
  
  mu_assert(ret_val == 0, "Failed final LCASuffixTree verification."); 

  return NULL;
}

char* test_long_run_allocs()
{
  char str[] = "AAAAAAAAAAAAAAAAAAAAAAAAA";
  size_t str_len = sizeof(str) - 1;

  LCASuffixTree_T tree = NULL;
  int i = 0;
  int ret_val = 1;
  for(i = 0; i < 250; i++){
    tree = LCASuffixTree_create(str, str_len);
    if(tree) {
      ret_val = LCASuffixTree_verify(tree);
    }
    LCASuffixTree_delete(&tree);
  }

  mu_assert(ret_val == 0, "Failed final LCASuffixTree verification.");

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  fprintf(stderr, "\n\nBEGIN LCASuffixTree_T ALLOC TESTS\n");
  mu_run_test(test_lca_suffix_tree_allocs);
  mu_run_test(test_long_run_allocs);
  fprintf(stderr, "END LCASuffixTree_T ALLOC TESTS\n\n");

  FREE_FAILING_ALLOCS

  return NULL;
}

RUN_TESTS(all_tests);
