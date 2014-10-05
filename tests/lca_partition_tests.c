#include "minunit.h"
#include "test_utils.h"
#include "lca/lca.h"

int verify_partition(size_t n)
{
  size_t* block_counts = calloc(get_num_partitions(n), sizeof(size_t));
  
  size_t i = 0;
  size_t last_block = 0;
  for(i = 0; i < n; i++) {
    size_t block = get_partition(i, n);
    if(!(block == last_block || block == last_block + 1)) {
      log_warn("Block assignments are not increasing.");
      return 1;
    }
    last_block = block;
    block_counts[block]++;
  }

  size_t running_position_sum = 0;
  for(i = 0; i < get_num_partitions(n); i++) {
    running_position_sum += block_counts[i];

    if(i != get_num_partitions(n) - 1) {
      if(block_counts[i] != get_partition_size(n)) {
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

char* test_partitioning()
{
  unsigned int i = 0;
  for(i = 100; i < 100000; i += 263) {
    int ret = verify_partition(i);
    mu_assert(ret == 0, "Failed partition verification.");
  }

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_partitioning);
  return NULL;
}

RUN_TESTS(all_tests);
