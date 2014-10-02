#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"
#include "lca/lca.h"

/*
 * Test that the number of nodes in the suffix tree for MISSISSIPPI is 18
 */
char* test_mississippi()
{
  char str[] = "MISSISSIPPI";
  size_t str_len = sizeof(str) - 1;
  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);

  DBL_WORD counter = 0;
  count_nodes(stree->root, &counter);
  
  mu_assert(counter == 19, "Invalid node count.");
  
  ST_DeleteTree(stree);

  return NULL;
}
  
/*
 * Test that the number of nodes in the suffix tree for BANANA is 10
 */
char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);

  DBL_WORD counter = 0;
  count_nodes(stree->root, &counter);
  
  mu_assert(counter == 11, "Invalid node count.");

  ST_DeleteTree(stree);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_mississippi);
  mu_run_test(test_banana);

  return NULL;
}

RUN_TESTS(all_tests);
