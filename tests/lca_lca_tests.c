#include "minunit.h"
#include "test_utils.h"

#include "lca/lca.h"

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  SuffixTree_T stree = SuffixTree_create(str, str_len);
  Node_T* node_array = SuffixTree_create_node_array(stree);
  
  TreeLCA* tree_lca = TreeLCA_create(stree);
  
  Node_T lca = TreeLCA_lookup(tree_lca, node_array[9], node_array[7]);
  mu_assert(Node_get_index(lca) == 7, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            9, 7, 7, Node_get_index(lca));

  lca = TreeLCA_lookup(tree_lca, node_array[7], node_array[9]);
  mu_assert(Node_get_index(lca) == 7, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            7, 9, 7, Node_get_index(lca));

  lca = TreeLCA_lookup(tree_lca, node_array[4], node_array[6]);
  mu_assert(Node_get_index(lca) == 2, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            4, 6, 2, Node_get_index(lca));

  lca = TreeLCA_lookup(tree_lca, node_array[8], node_array[5]);
  mu_assert(Node_get_index(lca) == 0, "LCA of nodes %d and %d should be %d, but we got %zu.\n",
            8, 5, 0, Node_get_index(lca));

  free(node_array);
  SuffixTree_delete(&stree);
  TreeLCA_delete(tree_lca);

  return NULL;
}

char* test_random()
{
  const size_t str_len = 700;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;

  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    TreeLCA* tree_lca = TreeLCA_create(stree);

    int ret = TreeLCA_verify(stree, tree_lca);
    
    mu_assert(ret == 0, "Failed random string LCA verification.");

    TreeLCA_delete(tree_lca);
    SuffixTree_delete(&stree);
  }
  free(str);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_banana);
  mu_run_test(test_random);

  return NULL;
}

RUN_TESTS(all_tests);
