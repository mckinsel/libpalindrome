#include "minunit.h"
#include "test_utils.h"
#include "lca/lca.h"

/* Test minima identification for a simple array of length 10. Partitions should
 * be of length 2.
 */
char* test_ten()
{

  size_t test_array[10] = {9, 0, 5, 9, 1000, 10000, 0, 1, 3, 2};

  size_t* block_minima = NULL;
  size_t* minima_pos = NULL;
  
  get_partition_minima(test_array, 10, &block_minima, &minima_pos);

  mu_assert(block_minima[0] == 0, "Incorrect block minimum for block %d.", 0);
  mu_assert(block_minima[1] == 5, "Incorrect block minimum for block %d.", 1);
  mu_assert(block_minima[2] == 1000, "Incorrect block minimum for block %d.", 2);
  mu_assert(block_minima[3] == 0, "Incorrect block minimum for block %d.", 3);
  mu_assert(block_minima[4] == 2, "Incorrect block minimum for block %d.", 4);
  
  mu_assert(minima_pos[0] == 1, "Incorrect minimum position for block %d.", 0); 
  mu_assert(minima_pos[1] == 0, "Incorrect minimum position for block %d.", 1); 
  mu_assert(minima_pos[2] == 0, "Incorrect minimum position for block %d.", 2); 
  mu_assert(minima_pos[3] == 0, "Incorrect minimum position for block %d.", 3); 
  mu_assert(minima_pos[4] == 1, "Incorrect minimum position for block %d.", 4); 
  
  free(block_minima);
  free(minima_pos);

  return NULL;
}

/* Test minima identification for a simple array of length 20. Partitions should
 * be of length 3.
 */
char* test_twenty()
{

  size_t test_array[20] = {11, 1, 7,
                           6, 14, 8,
                           1000, 100, 10000,
                           1, 1, 1,
                           0, 0, 0,
                           30, 20, 10,
                           16, 17};
  size_t* block_minima = NULL;
  size_t* minima_pos  = NULL;

  get_partition_minima(test_array, 20, &block_minima, &minima_pos);

  mu_assert(block_minima[0] == 1, "Incorrect block minimum for block %d.", 0);
  mu_assert(block_minima[1] == 6, "Incorrect block minimum for block %d.", 1);
  mu_assert(block_minima[2] == 100, "Incorrect block minimum for block %d.", 2);
  mu_assert(block_minima[3] == 1, "Incorrect block minimum for block %d.", 3);
  mu_assert(block_minima[4] == 0, "Incorrect block minimum for block %d.", 4);
  mu_assert(block_minima[5] == 10, "Incorrect block minimum for block %d.", 5);
  mu_assert(block_minima[6] == 16, "Incorrect block minimum for block %d.", 6);

  mu_assert(minima_pos[0] == 1, "Incorrect minimum position for block %d.", 0); 
  mu_assert(minima_pos[1] == 0, "Incorrect minimum position for block %d.", 1); 
  mu_assert(minima_pos[2] == 1, "Incorrect minimum position for block %d.", 2); 
  mu_assert(minima_pos[3] == 0, "Incorrect minimum position for block %d.", 3); 
  mu_assert(minima_pos[4] == 0, "Incorrect minimum position for block %d.", 4); 
  mu_assert(minima_pos[5] == 2, "Incorrect minimum position for block %d.", 5); 
  mu_assert(minima_pos[6] == 0, "Incorrect minimum position for block %d.", 6); 
  
  free(block_minima);
  free(minima_pos);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();  

  mu_run_test(test_ten);
  mu_run_test(test_twenty);

  return NULL;
}

RUN_TESTS(all_tests);
