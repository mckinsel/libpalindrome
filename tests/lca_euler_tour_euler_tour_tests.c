#include "minunit.h"
#include "test_utils.h"

#include "lca/euler_tour.h"
/* Test a string with unique suffixes. Depths should be 0, [1, 0]* */
char* test_abcde()
{
  char str[] = "ABCDE";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
  
  DBL_WORD* tour;
  DBL_WORD* depths;
  DBL_WORD* first_instances;
  
  euler_tour_arrays_create(stree, &tour, &depths, &first_instances);
  size_t root_id = stree->root->index;

  size_t i = 0;
  for(i = 0; i < 2 * stree->num_nodes - 1; i++) {
    if(i % 2 == 0) mu_assert(depths[i] == 0,
                             "Even positions in ABCDE are not root depth");
    if(i % 2 == 1) mu_assert(depths[i] == 1,
                             "Odd positions in ABCDE are not depth one.");

    if(i % 2 == 0) mu_assert(tour[i] == root_id,
                             "Even position in ABCDE tour is not the root.");
    if(i % 2 != 0) mu_assert(tour[i] != root_id,
                             "Odd position in ABCDE tour is the root.");
  }
  for(i = 0; i < stree->num_nodes; i++) {
    if(i == 0) {
      mu_assert(first_instances[i] == 0, "Incorrect first node instance assignment.");
    } else {
      mu_assert(first_instances[i] == i * 2 - 1,
                "Incorrect first node instance assignment.");
    }
  }

  int ret = verify_rmq_arrays(stree, tour, depths, first_instances);
  mu_assert(ret == 0, "RMQ array verification failed.");

  ST_DeleteTree(stree);
  free(tour);
  free(depths);
  free(first_instances);
  return NULL;
}

char* test_banana()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;

  SUFFIX_TREE* stree = ST_CreateTree(str, str_len);

  DBL_WORD* tour;
  DBL_WORD* depths;
  DBL_WORD* first_instances;
 
  euler_tour_arrays_create(stree, &tour, &depths, &first_instances);
  
  int ret = verify_rmq_arrays(stree, tour, depths, first_instances);
  mu_assert(ret == 0, "RMQ array verification failed.");
  
  ST_DeleteTree(stree);
  free(tour);
  free(depths);
  free(first_instances);
  return NULL;
}

char* test_random()
{
  const size_t str_len = 5000;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;
  for(i = 0; i < 25; i++) {
    random_string(str, str_len);
    SUFFIX_TREE* stree = ST_CreateTree(str, str_len);
    DBL_WORD* tour;
    DBL_WORD* depths;
    DBL_WORD* first_instances;
    euler_tour_arrays_create(stree, &tour, &depths, &first_instances);
    
    int ret = verify_rmq_arrays(stree, tour, depths, first_instances);
    mu_assert(ret == 0, "Failed RMQ array verification.");

    ST_DeleteTree(stree);
    free(tour);
    free(depths);
    free(first_instances);
  }
  free(str);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_abcde);
  mu_run_test(test_banana);
  mu_run_test(test_random);

  return NULL;
}

RUN_TESTS(all_tests);