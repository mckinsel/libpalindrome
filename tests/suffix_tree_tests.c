#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"

/* Test suffix tree construction on some random strings. */
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

/* Print a suffix tree. */
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

/* Helper function for testing node indices. */
int label_test_dfs(const NODE* node, DBL_WORD** label_record, DBL_WORD num_nodes)
{
  DBL_WORD index = node->index;

  if(index >= num_nodes) {
    log_warn("Node has index value greater than the number of nodes.");
    return 1;
  }

  if((*label_record)[index] != 0) {
    log_warn("Node index %zu appears multiple times in suffix tree.", index);
    return 1;
  }

  (*label_record)[index] = 1;

  NODE* next_node = node->sons;
  int ret = 0;
  while(next_node != 0) {
    ret = label_test_dfs(next_node, label_record, num_nodes);

    if(ret != 0) return ret;

    next_node = next_node->right_sibling;
  }

  return 0;
}

/* Test the index labels for a suffix tree for BANANA. */
char* test_node_labels_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
  DBL_WORD* label_record = calloc(stree->num_nodes, sizeof(DBL_WORD));

  int ret = label_test_dfs(stree->root, &label_record, stree->num_nodes);

  mu_assert(ret == 0, "Failed node index verfication for BANANA.");
  DBL_WORD i = 0;
  for(i = 0; i < stree->num_nodes; i++) {
    mu_assert(label_record[i] == 1, "Node index not visited during tour.");
  }
  
  free(label_record);
  ST_DeleteTree(stree);

  return  NULL;
}

/* Test the index labels for a suffix tree for some random string. */
char* test_node_labels_random()
{
  const size_t str_len = 10000;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;

  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
    DBL_WORD* label_record = calloc(stree->num_nodes, sizeof(DBL_WORD));

    int ret = label_test_dfs(stree->root, &label_record, stree->num_nodes);

    mu_assert(ret == 0, "Failed node index verfication for random string.");
    DBL_WORD i = 0;
    for(i = 0; i < stree->num_nodes; i++) {
      mu_assert(label_record[i] == 1, "Node index not visited during tour.");
    }
    
    free(label_record);
    ST_DeleteTree(stree);
  }

  free(str);

  return NULL;
}

char* test_node_array()
{
  const size_t str_len = 10000;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;

  for(i = 0; i < 5; i++) {
    random_string(str, str_len);
    SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
    
    NODE** node_array = ST_CreateNodeArray(stree);

    size_t j = 0;
    for(j = 0; j < stree->num_nodes; j++) {
      mu_assert(node_array[i]->index == i, "Incorrect node array assignment.");
    }
    free(node_array);
    ST_DeleteTree(stree);
  }
  
  free(str);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_random_strings);
  mu_run_test(test_print_stree);
  mu_run_test(test_node_labels_banana);
  mu_run_test(test_node_labels_random);
  mu_run_test(test_node_array);

  return NULL;
}

RUN_TESTS(all_tests);
