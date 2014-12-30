#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"


char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T tree = SuffixTree_create(str, str_len);
  SuffixTree_print(tree);

  Node_T* leaf_array = SuffixTree_create_leaf_array(tree);
  
  int ret = SuffixTree_verify_leaf_array(tree, leaf_array);
  mu_assert(ret == 0, "Failed position to leaf verification.");

  free(leaf_array);
  SuffixTree_delete(&tree);

  return NULL;
}

char* test_mississippi()
{
  char str[] = "MISSISSIPPI";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T tree = SuffixTree_create(str, str_len);
  Node_T* leaf_array = SuffixTree_create_leaf_array(tree);
  
  int ret = SuffixTree_verify_leaf_array(tree, leaf_array);
  mu_assert(ret == 0, "Failed position to leaf verification.");

  free(leaf_array);
  SuffixTree_delete(&tree);

  return NULL;
}

char* test_random_strings()
{
  const size_t str_len = 5000;
  char* str = calloc((str_len + 1), sizeof(char));

  unsigned int i = 0;
  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    SuffixTree_T tree = SuffixTree_create(str, str_len);
    Node_T* leaf_array = SuffixTree_create_leaf_array(tree);

    int ret = SuffixTree_verify_leaf_array(tree, leaf_array);
    mu_assert(ret == 0, "Failed leaf_array verification for random string.");

    free(leaf_array);
    SuffixTree_delete(&tree);
  }
  
  free(str);
  return NULL;
}

char* test_verification()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T tree = SuffixTree_create(str, str_len);
  Node_T* leaf_array = SuffixTree_create_leaf_array(tree);
  
  Node_T tmp = NULL;
  tmp = leaf_array[3];
  leaf_array[3] = leaf_array[4];
  leaf_array[4] = tmp;
  
  fprintf(stderr, "Expect pos to map verification warning:\n"); 
  int ret = SuffixTree_verify_leaf_array(tree, leaf_array);
  mu_assert(ret == 1, "Verified an incorrect leaf_map.");

  free(leaf_array);
  SuffixTree_delete(&tree);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_banana);
  mu_run_test(test_mississippi);
  mu_run_test(test_random_strings);
  mu_run_test(test_verification);

  return NULL;
}

RUN_TESTS(all_tests);
