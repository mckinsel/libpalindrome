#include "failing_allocs.h"
#include "minunit.h"

#include "suffix_tree/suffix_tree.h"

USE_FAILING_ALLOCS

char* test_suffix_tree_allocs()
{
  fprintf(stderr, "\n\nBEGIN SuffixTree_T ALLOC TESTS\n");
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  int i = 0;
  for(i = 0; i < 25; i++) {
    SuffixTree_T tree = SuffixTree_create(str, str_len);
    if(tree) {
      int rc = SuffixTree_verify(tree);
      mu_assert(rc == 0, "Failed suffix tree verification.");

      Node_T* node_array = SuffixTree_create_node_array(tree);
      if(node_array) {
        free(node_array);
      } else {
        printf("Node array creation failed.\n");
      }
      Node_T* leaf_array = SuffixTree_create_leaf_array(tree);
      if(leaf_array) {
        free(leaf_array);
      } else {
        printf("Leaf array creation failed.\n");
      }
      
    }
    SuffixTree_delete(&tree);
  }

  fprintf(stderr, "END SuffixTree_T ALLOC TESTS\n\n");
  return NULL;
}


char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_suffix_tree_allocs);
  
  FREE_FAILING_ALLOCS

  return NULL;
}

RUN_TESTS(all_tests);
