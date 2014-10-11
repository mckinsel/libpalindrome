#include "minunit.h"
#include "test_utils.h"

#include "lca/lca.h"

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);

  TreeLCA* tree_lca = TreeLCA_create(stree);

  ST_DeleteTree(stree);
  TreeLCA_delete(tree_lca);

  return NULL;
}
char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_banana);

  return NULL;
}

RUN_TESTS(all_tests);
