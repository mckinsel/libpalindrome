#include "minunit.h"
#include "test_utils.h"

#include "lca/euler_tour.h"
/* Test a string with unique suffixes. Depths should be 0, [1, 0]* */
char* test_abcde()
{
  char str[] = "ABCDE";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T stree = SuffixTree_create(str, str_len);
  EulerTour_T euler_tour = EulerTour_create(stree);
  
  SuffixTree_print(stree);
  EulerTour_print(euler_tour);

  SuffixTreeIndex_T root_id = Node_get_index(SuffixTree_get_root(stree));
  SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes(stree);

  size_t i = 0;
  for(i = 0; i < 2 * num_nodes - 1; i++) {
    if(i % 2 == 0) mu_assert(euler_tour->depths[i] == 0,
                             "Even positions in ABCDE are not root depth");
    if(i % 2 == 1) mu_assert(euler_tour->depths[i] == 1,
                             "Odd positions in ABCDE are not depth one.");

    if(i % 2 == 0) mu_assert(Node_get_index(euler_tour->nodes[i]) == root_id,
                             "Even position in ABCDE tour is not the root.");
    if(i % 2 != 0) mu_assert(Node_get_index(euler_tour->nodes[i]) != root_id,
                             "Odd position in ABCDE tour is the root.");
  }
  for(i = 0; i < num_nodes; i++) {
    if(i == 0) {
      mu_assert(euler_tour->first_instances[i] == 0, "Incorrect first node instance assignment.");
    } else {
      mu_assert(euler_tour->first_instances[i] == i * 2 - 1,
                "Incorrect first node instance assignment.");
    }
  }

  int ret = EulerTour_verify(euler_tour, stree);
  mu_assert(ret == 0, "Euler tour verification failed.");

  SuffixTree_delete(&stree);
  EulerTour_delete(&euler_tour);
  return NULL;
}

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T stree = SuffixTree_create(str, str_len);

  EulerTour_T euler_tour = EulerTour_create(stree);
  
  int ret = EulerTour_verify(euler_tour, stree);
  mu_assert(ret == 0, "Euler tour verification failed.");
  
  SuffixTree_delete(&stree);
  EulerTour_delete(&euler_tour);
  return NULL;
}

char* test_random()
{
  const size_t str_len = 5000;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;
  for(i = 0; i < 25; i++) {
    random_string(str, str_len);
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    EulerTour_T euler_tour = EulerTour_create(stree);
    
    int ret = EulerTour_verify(euler_tour, stree);
    mu_assert(ret == 0, "Euler tour verification failed.");

    SuffixTree_delete(&stree);
    EulerTour_delete(&euler_tour);
  }
  free(str);
  return NULL;
}

char* test_verification()
{
  char str[] = "MISSISSIPPI";
  size_t str_len = sizeof(str) - 1;

  SuffixTree_T tree = SuffixTree_create(str, str_len);
  EulerTour_T euler_tour = EulerTour_create(tree);

  SuffixTreeIndex_T* good_depths = calloc(euler_tour->length,
                                          sizeof(SuffixTreeIndex_T));
  memcpy(good_depths, euler_tour->depths, euler_tour->length*sizeof(SuffixTreeIndex_T));
  euler_tour->depths[2] = euler_tour->depths[1] + 2;
  int ret_val = EulerTour_verify(euler_tour, tree);
  mu_assert(ret_val != 0, "Euler tour verification succeeded for incorrect depths.");
  memcpy(euler_tour->depths, good_depths, euler_tour->length*sizeof(SuffixTreeIndex_T));
  free(good_depths);

  Node_T tmp_root = euler_tour->nodes[0];
  euler_tour->nodes[0] = euler_tour->nodes[1];
  ret_val = EulerTour_verify(euler_tour, tree);
  mu_assert(ret_val != 0, "Euler tour verification succeeded for incorrect first node.");
  euler_tour->nodes[0] = tmp_root;

  euler_tour->nodes[euler_tour->length - 1] = euler_tour->nodes[1];
  ret_val = EulerTour_verify(euler_tour, tree);
  mu_assert(ret_val != 0, "Euler tour verification succeeded for incorrect last node.");
  euler_tour->nodes[euler_tour->length - 1] = tmp_root;

  size_t tmp_first_instance = euler_tour->first_instances[1];
  euler_tour->first_instances[1] = euler_tour->first_instances[0];
  euler_tour->first_instances[0] = tmp_first_instance;
  ret_val = EulerTour_verify(euler_tour, tree);
  mu_assert(ret_val != 0, "Euler tour verification succeeded for incorrect first instances.");
  euler_tour->first_instances[0] = euler_tour->first_instances[1];
  euler_tour->first_instances[1] = tmp_first_instance;

  EulerTour_delete(&euler_tour);
  SuffixTree_delete(&tree);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_abcde);
  mu_run_test(test_banana);
  mu_run_test(test_random);
  mu_run_test(test_verification);

  return NULL;
}

RUN_TESTS(all_tests);
