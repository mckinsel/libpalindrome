#include "minunit.h"
#include "test_utils.h"
#include "lca/tour_partition.h"

char* test_get_block()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);

  TourPartition_T tour_partition = TourPartition_create(arr, arr_size);
  
  size_t* block = calloc(2, sizeof(size_t));
  size_t block_size = 0;

  block_size = TourPartition_get_block(&block, tour_partition, 0, arr);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 5, "Incorrect block returned.");
  mu_assert(block[1] == 4, "Incorrect block returned.");

  block_size = TourPartition_get_block(&block, tour_partition, 1, arr);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 3, "Incorrect block returned.");
  mu_assert(block[1] == 2, "Incorrect block returned.");

  block_size = TourPartition_get_block(&block, tour_partition, 2, arr);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 1, "Incorrect block returned.");
  mu_assert(block[1] == 2, "Incorrect block returned.");

  block_size = TourPartition_get_block(&block, tour_partition, 3, arr);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 3, "Incorrect block returned.");
  mu_assert(block[1] == 4, "Incorrect block returned.");
  
  block_size = TourPartition_get_block(&block, tour_partition, 4, arr);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 5, "Incorrect block returned.");
  mu_assert(block[1] == 6, "Incorrect block returned.");

  block_size = TourPartition_get_block(&block, tour_partition, 5, arr);
  mu_assert(block_size == 1, "Incorrect returned block size");
  mu_assert(block[0] == 7, "Incorrect block returned.");
  
  free(block); 
  TourPartition_delete(&tour_partition);

  return NULL;
}

char* test_pos_in_block()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);

  TourPartition_T tour_partition = TourPartition_create(arr, arr_size);

  size_t pos = 0;

  pos = TourPartition_get_pos_in_block(tour_partition, 0);
  mu_assert(pos == 0, "Incorrect pos in block.");

  pos = TourPartition_get_pos_in_block(tour_partition, 1);
  mu_assert(pos == 1, "Incorrect pos in block.");

  pos = TourPartition_get_pos_in_block(tour_partition, 2);
  mu_assert(pos == 0, "Incorrect pos in block.");

  pos = TourPartition_get_pos_in_block(tour_partition, 7);
  mu_assert(pos == 1, "Incorrect pos in block.");

  pos = TourPartition_get_pos_in_block(tour_partition, 10);
  mu_assert(pos == 0, "Incorrect pos in block.");

  TourPartition_delete(&tour_partition);
  return NULL;
}

char* test_partitioning()
{
  unsigned int i = 0;
  size_t* arr = calloc(100000, sizeof(size_t));
  for(i = 100; i < 100000; i += 263) {
    TourPartition_T tour_partition = TourPartition_create(arr, i);
    int ret = TourPartition_verify(tour_partition, arr, i);
    mu_assert(ret == 0, "Failed block verification.");
    TourPartition_delete(&tour_partition);
  }
  free(arr);

  return NULL;
}

char* test_block_minima()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);
  
  TourPartition_T tour_partition = TourPartition_create(arr, arr_size);

  mu_assert(tour_partition->num_blocks == 6, "Incorrect number of blocks.");
  
  mu_assert(tour_partition->block_minima[0] == 4, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[0] == 1, "Incorrect minimum position.");
  
  mu_assert(tour_partition->block_minima[1] == 2, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[1] == 1, "Incorrect minimum position.");
  
  mu_assert(tour_partition->block_minima[2] == 1, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[2] == 0, "Incorrect minimum position.");

  mu_assert(tour_partition->block_minima[3] == 3, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[3] == 0, "Incorrect minimum position.");

  mu_assert(tour_partition->block_minima[4] == 5, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[4] == 0, "Incorrect minimum position.");

  mu_assert(tour_partition->block_minima[5] == 7, "Incorrect block minimum.");
  mu_assert(tour_partition->minima_positions[5] == 0, "Incorrect minimum position.");
  
  TourPartition_delete(&tour_partition);

  return NULL;
}

char* test_boundaries()
{
  size_t* arr1 = NULL;
  size_t arr_size = 0;

  TourPartition_T tour_partition = TourPartition_create(arr1, arr_size);
  mu_assert(tour_partition == NULL, "TourPartition tried to create an empty partition.");

  size_t arr2[1] = {5};
  arr_size = sizeof(arr1)/sizeof(size_t);
  tour_partition = TourPartition_create(arr2, arr_size);

  mu_assert(TourPartition_get_block_index(tour_partition, 0) == 0,
            "Incorrect block index for partition of size one array.");
  mu_assert(TourPartition_get_pos_in_block(tour_partition, 0) == 0,
            "Incorrect pos in block for partition of size one array.");

  TourPartition_delete(&tour_partition);

  return NULL;
}

char* test_verification()
{
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);

  TourPartition_T tour_partition = TourPartition_create(arr, arr_size);

  size_t good_total_length = tour_partition->total_length;
  tour_partition->total_length = 3;

  int ret_val = TourPartition_verify(tour_partition, arr, arr_size);
  mu_assert(ret_val != 0,
            "TourPartition verification succeeded on partition with incorrect total_length.");
  tour_partition->total_length = good_total_length;

  tour_partition->block_minima[0] = 99;
  ret_val = TourPartition_verify(tour_partition, arr, arr_size);
  mu_assert(ret_val != 0,
            "TourPartition verification succeeded on partition with incorrect block_minima.");
  tour_partition->block_minima[0] = 4;

  return NULL;
}


char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_partitioning);
  mu_run_test(test_get_block);
  mu_run_test(test_pos_in_block);
  mu_run_test(test_block_minima);
  mu_run_test(test_boundaries);
  mu_run_test(test_verification);

  return NULL;
}

RUN_TESTS(all_tests);
