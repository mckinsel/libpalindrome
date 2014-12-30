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
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    int rc = SuffixTree_verify(stree);

    mu_assert(rc == 0, "Suffix tree failed self test.");

    SuffixTree_delete(&stree);
  }

  free(str);

  return NULL;
}

/* Print a suffix tree. */
char* test_print_stree()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T stree = SuffixTree_create(str, str_len);
  printf("\nSuffix tree for BANANA:");
  SuffixTree_print(stree);
  int rc = SuffixTree_verify(stree);

  mu_assert(rc == 0, "Suffix tree failed self test.");
    
  SuffixTree_delete(&stree);

  return NULL;
}

/* Helper function for testing node indices. */
int label_test_dfs(Node_T node, SuffixTreeIndex_T** label_record, SuffixTreeIndex_T num_nodes)
{
  SuffixTreeIndex_T index = Node_get_index(node);

  if(index >= num_nodes) {
    log_warn("Node has index value greater than the number of nodes.");
    return 1;
  }

  if((*label_record)[index] != 0) {
    log_warn("Node index %zu appears multiple times in suffix tree.", index);
    return 1;
  }

  (*label_record)[index] = 1;

  Node_T next_node = Node_get_child(node);
  int ret = 0;
  while(next_node != 0) {
    ret = label_test_dfs(next_node, label_record, num_nodes);

    if(ret != 0) return ret;

    next_node = Node_get_sibling(next_node);
  }

  return 0;
}

/* Test the index labels for a suffix tree for BANANA. */
char* test_node_labels_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T stree = SuffixTree_create(str, str_len);
  SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes(stree);
  SuffixTreeIndex_T* label_record = calloc(num_nodes, sizeof(SuffixTreeIndex_T));

  int ret = label_test_dfs(SuffixTree_get_root(stree), &label_record, num_nodes);

  mu_assert(ret == 0, "Failed node index verfication for BANANA.");
  SuffixTreeIndex_T i = 0;
  for(i = 0; i < num_nodes; i++) {
    mu_assert(label_record[i] == 1, "Node index not visited during tour.");
  }
  
  free(label_record);
  SuffixTree_delete(&stree);

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
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes(stree);
    SuffixTreeIndex_T* label_record = calloc(num_nodes, sizeof(SuffixTreeIndex_T));

    int ret = label_test_dfs(SuffixTree_get_root(stree), &label_record, num_nodes);

    mu_assert(ret == 0, "Failed node index verfication for random string.");
    SuffixTreeIndex_T i = 0;
    for(i = 0; i < num_nodes; i++) {
      mu_assert(label_record[i] == 1, "Node index not visited during tour.");
    }
    
    free(label_record);
    SuffixTree_delete(&stree);
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
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes(stree);
    
    Node_T* node_array = SuffixTree_create_node_array(stree);

    size_t j = 0;
    for(j = 0; j < num_nodes; j++) {
      mu_assert(Node_get_index(node_array[i]) == i, "Incorrect node array assignment.");
    }
    free(node_array);
    SuffixTree_delete(&stree);
  }
  
  free(str);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_print_stree);
  mu_run_test(test_random_strings);
  mu_run_test(test_node_labels_banana);
  mu_run_test(test_node_labels_random);
  mu_run_test(test_node_array);

  return NULL;
}

RUN_TESTS(all_tests);
