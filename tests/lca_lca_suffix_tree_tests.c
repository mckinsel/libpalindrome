#include "minunit.h"
#include "test_utils.h"

#include "lca/lca_suffix_tree.h"

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  LCASuffixTree_T tree = LCASuffixTree_create(str, str_len);
  int retval = LCASuffixTree_verify(tree);
  mu_assert(retval == 0, "Failed LCASuffixTree verification for BANANA.");

  SuffixTree_print((SuffixTree_T)tree);
  Node_T* leaf_array = SuffixTree_create_leaf_array((SuffixTree_T)tree);
  size_t i = 0;
  for(i = 0; i < str_len; i++) {
    Node_T leaf = leaf_array[i];
    printf("Position: %zu Node index: %zu\n", i, Node_get_index(leaf));
  }
  free(leaf_array);

  Node_T* node_array = SuffixTree_create_node_array((SuffixTree_T)tree);
  
  Node_T lca = LCASuffixTree_get_lca(tree, node_array[9], node_array[7]);
  mu_assert(Node_get_index(lca) == 7, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            9, 7, 7, Node_get_index(lca));

  lca = LCASuffixTree_get_lca(tree, node_array[7], node_array[9]);
  mu_assert(Node_get_index(lca) == 7, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            7, 9, 7, Node_get_index(lca));

  lca = LCASuffixTree_get_lca(tree, node_array[4], node_array[6]);
  mu_assert(Node_get_index(lca) == 2, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            4, 6, 2, Node_get_index(lca));

  lca = LCASuffixTree_get_lca(tree, node_array[8], node_array[5]);
  mu_assert(Node_get_index(lca) == 0, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            8, 5, 0, Node_get_index(lca));

  free(node_array);
  LCASuffixTree_delete(&tree);

  return NULL;
}

char* test_banana_with_reverse()
{
  char str[] = "BANANA#ANANAB";
  size_t str_len = sizeof(str) - 1;
  LCASuffixTree_T tree = LCASuffixTree_create(str, str_len);

  SuffixTree_print((SuffixTree_T)tree);
  Node_T* leaf_array = SuffixTree_create_leaf_array((SuffixTree_T)tree);
  size_t i = 0;
  for(i = 0; i < str_len; i++) {
    Node_T leaf = leaf_array[i];
    printf("Position: %zu Node index: %zu\n", i, Node_get_index(leaf));
  }
  free(leaf_array);

  LCASuffixTree_delete(&tree);

  return NULL;
}
char* test_random()
{
  const size_t str_len = 700;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;

  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    LCASuffixTree_T tree = LCASuffixTree_create(str, str_len);

    int ret = LCASuffixTree_verify(tree);
    
    mu_assert(ret == 0, "Failed random string LCA verification.");

    LCASuffixTree_delete(&tree);
  }
  free(str);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_banana);
  mu_run_test(test_banana_with_reverse);
  mu_run_test(test_random);

  return NULL;
}

RUN_TESTS(all_tests);
