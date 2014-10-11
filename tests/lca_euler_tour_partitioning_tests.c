#include "minunit.h"
#include "test_utils.h"
#include "lca/euler_tour.h"

/*
 * Run tests for the partitioning of an array of size n.
 */
int verify_partitioning(size_t n)
{
  size_t* block_counts = calloc(get_num_blocks(n), sizeof(size_t));
  
  size_t i = 0;
  size_t last_block = 0;
  for(i = 0; i < n; i++) {
    size_t block = get_block_index(i, n);
    if(!(block == last_block || block == last_block + 1)) {
      log_warn("Block assignments are not increasing.");
      return 1;
    }
    last_block = block;
    block_counts[block]++;
  }

  size_t running_position_sum = 0;
  for(i = 0; i < get_num_blocks(n); i++) {
    running_position_sum += block_counts[i];

    if(i != get_num_blocks(n) - 1) {
      if(block_counts[i] != get_block_size(n)) {
        log_warn("Non-final block has an incorrect size.");
        return 1;
      }
    } else {
      if(block_counts[i] == 0) {
        log_warn("Empty final block.");
        return 1;
      }
    }
  }

  if(running_position_sum != n) {
    log_warn("Total block assignments does not equal size of input array.");
    return 1;
  }

  free(block_counts);
  return  0;
}

char* test_get_block()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);
  
  size_t* block = calloc(2, sizeof(size_t));
  size_t block_size = 0;

  block_size = get_block(&block, 0, arr, arr_size);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 5, "Incorrect block returned.");
  mu_assert(block[1] == 4, "Incorrect block returned.");

  block_size = get_block(&block, 1, arr, arr_size);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 3, "Incorrect block returned.");
  mu_assert(block[1] == 2, "Incorrect block returned.");

  block_size = get_block(&block, 2, arr, arr_size);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 1, "Incorrect block returned.");
  mu_assert(block[1] == 2, "Incorrect block returned.");

  block_size = get_block(&block, 3, arr, arr_size);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 3, "Incorrect block returned.");
  mu_assert(block[1] == 4, "Incorrect block returned.");
  
  block_size = get_block(&block, 4, arr, arr_size);
  mu_assert(block_size == 2, "Incorrect returned block size");
  mu_assert(block[0] == 5, "Incorrect block returned.");
  mu_assert(block[1] == 6, "Incorrect block returned.");

  block_size = get_block(&block, 5, arr, arr_size);
  mu_assert(block_size == 1, "Incorrect returned block size");
  mu_assert(block[0] == 7, "Incorrect block returned.");
  
  free(block); 
  return NULL;
}

char* test_pos_in_block()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);
  size_t pos = 0;

  pos = get_pos_in_block(0, arr_size);
  mu_assert(pos == 0, "Incorrect pos in block.");

  pos = get_pos_in_block(1, arr_size);
  mu_assert(pos == 1, "Incorrect pos in block.");

  pos = get_pos_in_block(2, arr_size);
  mu_assert(pos == 0, "Incorrect pos in block.");

  pos = get_pos_in_block(7, arr_size);
  mu_assert(pos == 1, "Incorrect pos in block.");

  pos = get_pos_in_block(10, arr_size);
  mu_assert(pos == 0, "Incorrect pos in block.");
  return NULL;
}

char* test_partitioning()
{
  unsigned int i = 0;
  for(i = 100; i < 100000; i += 263) {
    int ret = verify_partitioning(i);
    mu_assert(ret == 0, "Failed block verification.");
  }

  return NULL;
}

char* test_block_minima()
{
                 /* 0  1  2  3  4  5  6  7  8  9  0 */
  size_t arr[11] = {5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};
  size_t arr_size = sizeof(arr)/sizeof(size_t);
  
  size_t* block_minima = NULL;
  size_t* minima_positions = NULL;

  size_t num_blocks = get_block_minima(arr, arr_size, &block_minima,
                                       &minima_positions);

  mu_assert(num_blocks == 6, "Incorrect number of blocks.");
  
  mu_assert(block_minima[0] == 4, "Incorrect block minimum."); 
  mu_assert(minima_positions[0] == 1, "Incorrect minimum position."); 
  
  mu_assert(block_minima[1] == 2, "Incorrect block minimum."); 
  mu_assert(minima_positions[1] == 1, "Incorrect minimum position."); 
  
  mu_assert(block_minima[2] == 1, "Incorrect block minimum."); 
  mu_assert(minima_positions[2] == 0, "Incorrect minimum position."); 

  mu_assert(block_minima[3] == 3, "Incorrect block minimum."); 
  mu_assert(minima_positions[3] == 0, "Incorrect minimum position."); 

  mu_assert(block_minima[4] == 5, "Incorrect block minimum."); 
  mu_assert(minima_positions[4] == 0, "Incorrect minimum position."); 

  mu_assert(block_minima[5] == 7, "Incorrect block minimum."); 
  mu_assert(minima_positions[5] == 0, "Incorrect minimum position."); 
  
  free(block_minima);
  free(minima_positions);
  return NULL;
}
char* all_tests()
{
  mu_suite_start();

  mu_run_test(test_partitioning);
  mu_run_test(test_get_block);
  mu_run_test(test_pos_in_block);
  mu_run_test(test_block_minima);

  return NULL;
}

RUN_TESTS(all_tests);
