#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"
#include "lca/lca.h"

char* test_mississippi()
{
  char str[] = "MISSISSIPPI";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
  NODE** pos_to_leaf = map_position_to_leaf(stree, str_len);
  
  int ret = verify_map_position_to_leaf(pos_to_leaf, stree, str_len);
  mu_assert(ret == 0, "Failed position to leaf verification.");

  free(pos_to_leaf);
  ST_DeleteTree(stree);

  return NULL;
}

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
  NODE** pos_to_leaf = map_position_to_leaf(stree, str_len);
  
  int ret = verify_map_position_to_leaf(pos_to_leaf, stree, str_len);
  mu_assert(ret == 0, "Failed position to leaf verification.");

  free(pos_to_leaf);
  ST_DeleteTree(stree);

  return NULL;
}

char* test_random_strings()
{
  const size_t str_len = 5000;
  char* str = calloc((str_len + 1), sizeof(char));

  unsigned int i = 0;
  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
    NODE** pos_to_leaf = map_position_to_leaf(stree, str_len);

    int ret = verify_map_position_to_leaf(pos_to_leaf, stree, str_len);
    mu_assert(ret == 0, "Failed pos_to_leaf verification for random string.");

    free(pos_to_leaf);
    ST_DeleteTree(stree);
  }
  
  free(str);
  return NULL;
}

char* test_verification()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
  NODE** pos_to_leaf = map_position_to_leaf(stree, str_len);
  
  NODE* tmp = NULL;
  tmp = pos_to_leaf[3];
  pos_to_leaf[3] = pos_to_leaf[4];
  pos_to_leaf[4] = tmp;
  
  fprintf(stderr, "Expect pos to map verification warning:\n"); 
  int ret = verify_map_position_to_leaf(pos_to_leaf, stree, str_len);
  mu_assert(ret == 1, "Verified an incorrect leaf_map.");

  free(pos_to_leaf);
  ST_DeleteTree(stree);

  return NULL;

}
char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_mississippi);
  mu_run_test(test_banana);
  mu_run_test(test_verification);
  mu_run_test(test_random_strings);

  return NULL;
}

RUN_TESTS(all_tests);
