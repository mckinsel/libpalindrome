#include "minunit.h"
#include "test_utils.h"
#include "lca/tour_partition.h"

/* Test minima identification for a simple array of length 10. Partitions should
 * be of length 2.
 */
char* test_ten()
{

  size_t test_array[10] = {9, 0, 5, 9, 1000, 10000, 0, 1, 3, 2};

  TourPartition_T tour_partition = TourPartition_create(test_array, 10);

  mu_assert(tour_partition->block_minima[0] == 0,
            "Incorrect block minimum for block %d.", 0);
  mu_assert(tour_partition->block_minima[1] == 5,
            "Incorrect block minimum for block %d.", 1);
  mu_assert(tour_partition->block_minima[2] == 1000,
            "Incorrect block minimum for block %d.", 2);
  mu_assert(tour_partition->block_minima[3] == 0,
            "Incorrect block minimum for block %d.", 3);
  mu_assert(tour_partition->block_minima[4] == 2,
            "Incorrect block minimum for block %d.", 4);
  
  mu_assert(tour_partition->minima_positions[0] == 1,
            "Incorrect minimum position for block %d.", 0);
  mu_assert(tour_partition->minima_positions[1] == 0,
            "Incorrect minimum position for block %d.", 1);
  mu_assert(tour_partition->minima_positions[2] == 0,
            "Incorrect minimum position for block %d.", 2);
  mu_assert(tour_partition->minima_positions[3] == 0,
            "Incorrect minimum position for block %d.", 3);
  mu_assert(tour_partition->minima_positions[4] == 1,
            "Incorrect minimum position for block %d.", 4);
  
  TourPartition_delete(&tour_partition);

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
  TourPartition_T tour_partition = TourPartition_create(test_array, 20);

  mu_assert(tour_partition->block_minima[0] == 1,
            "Incorrect block minimum for block %d.", 0);
  mu_assert(tour_partition->block_minima[1] == 6,
            "Incorrect block minimum for block %d.", 1);
  mu_assert(tour_partition->block_minima[2] == 100,
            "Incorrect block minimum for block %d.", 2);
  mu_assert(tour_partition->block_minima[3] == 1,
            "Incorrect block minimum for block %d.", 3);
  mu_assert(tour_partition->block_minima[4] == 0,
            "Incorrect block minimum for block %d.", 4);
  mu_assert(tour_partition->block_minima[5] == 10,
            "Incorrect block minimum for block %d.", 5);
  mu_assert(tour_partition->block_minima[6] == 16,
            "Incorrect block minimum for block %d.", 6);

  mu_assert(tour_partition->minima_positions[0] == 1,
            "Incorrect minimum position for block %d.", 0);
  mu_assert(tour_partition->minima_positions[1] == 0,
            "Incorrect minimum position for block %d.", 1);
  mu_assert(tour_partition->minima_positions[2] == 1,
            "Incorrect minimum position for block %d.", 2);
  mu_assert(tour_partition->minima_positions[3] == 0,
            "Incorrect minimum position for block %d.", 3);
  mu_assert(tour_partition->minima_positions[4] == 0,
            "Incorrect minimum position for block %d.", 4);
  mu_assert(tour_partition->minima_positions[5] == 2,
            "Incorrect minimum position for block %d.", 5);
  mu_assert(tour_partition->minima_positions[6] == 0,
            "Incorrect minimum position for block %d.", 6);
  
  TourPartition_delete(&tour_partition);

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
