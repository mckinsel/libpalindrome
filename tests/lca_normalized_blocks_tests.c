#include "minunit.h"
#include "test_utils.h"
#include "lca/normalized_blocks.h"

char* test_block_ids()
{ 
  size_t block[] = {1, 2, 3, 4};
  size_t block_size = sizeof(block)/sizeof(size_t);
  unsigned int block_id = get_block_id(block, block_size);
  mu_assert(block_id == 7, "Incorrect block_id: %u instead of %u", block_id, 7);

  block[0] = 5; block[1] = 6; block[2] = 7; block[3] = 8;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 7, "Incorrect block_id: %u instead of %u", block_id, 7);

  block[0] = 5; block[1] = 6; block[2] = 7; block[3] = 6;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 6, "Incorrect block_id: %u instead of %u", block_id, 6);

  block[0] = 5; block[1] = 6; block[2] = 5; block[3] = 6;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 5, "Incorrect block_id: %u instead of %u", block_id, 5);

  block[0] = 5; block[1] = 6; block[2] = 5; block[3] = 4;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 4, "Incorrect block_id: %u instead of %u", block_id, 4);
  
  block[0] = 5; block[1] = 4; block[2] = 5; block[3] = 6;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 3, "Incorrect block_id: %u instead of %u", block_id, 3);

  block[0] = 5; block[1] = 4; block[2] = 5; block[3] = 4;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 2, "Incorrect block_id: %u instead of %u", block_id, 2);

  block[0] = 5; block[1] = 4; block[2] = 3; block[3] = 4;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 1, "Incorrect block_id: %u instead of %u", block_id, 1);

  block[0] = 5; block[1] = 4; block[2] = 3; block[3] = 2;
  block_id = get_block_id(block, block_size);
  mu_assert(block_id == 0, "Incorrect block_id: %u instead of %u", block_id, 0);

  return NULL;
}

char* test_brt_lookup()
{
  size_t block[20] = {10, 11, 12, 13, 12, 11, 10, 11, 10,  9,
                    /* 0   1   2   3   4   5   6   7   8   9 */
                       8,  7,  6,  5,  6,  7,  8,  7,  6,  7}; 

  size_t block_size = sizeof(block)/sizeof(size_t);
  
  BlockRMQTable* block_rmq_table = BRT_create(block, block_size); 
  BRT_print(block_rmq_table);
  size_t ret = 0;

  ret = BRT_lookup(block_rmq_table, 5, 12);
  mu_assert(ret == 11, "BRT_lookup failed, expected %d but got %zu",
            11, ret);

  ret = BRT_lookup(block_rmq_table, 12, 5);
  mu_assert(ret == 11, "BRT_lookup failed, expected %d but got %zu",
            11, ret);

  ret = BRT_lookup(block_rmq_table, 1, 7);
  mu_assert(ret == 6, "BRT_lookup failed, expected %d but got %zu",
            6, ret);

  ret = BRT_lookup(block_rmq_table, 0, 20);
  mu_assert(ret == 13, "BRT_lookup failed, expected %d but got %zu",
            13, ret);

  ret = BRT_lookup(block_rmq_table, 20, 0);
  mu_assert(ret == 13, "BRT_lookup failed, expected %d but got %zu",
            13, ret);

  ret = BRT_lookup(block_rmq_table, 14, 19);
  mu_assert(ret == 14, "BRT_lookup failed, expected %d but got %zu",
            14, ret);

  BRT_delete(block_rmq_table);

  return NULL;
}

char* test_brt_lookup_random()
{

  size_t* block = calloc(50, sizeof(size_t));
  size_t block_size = 50;

  unsigned int i = 0;
  for(i = 0; i < 100; i++) {
    random_sizes(block, block_size);
    
    BlockRMQTable* block_rmq_table = BRT_create(block, block_size);
    
    int ret = BRT_verify(block_rmq_table, block);

    mu_assert(ret == 0, "BlockRMQTable verification failed on random block.");
    BRT_delete(block_rmq_table);
  }
  free(block);
  return NULL;

}

/* Test lookups in the BlockRMQDatabase agains some simple expected values. */
char* test_brd_lookup()
{ 
                    /* 0   1   2   3   4 */
  size_t block1[5] = { 5,  6,  7,  6,  7};
  size_t block2[5] = {19, 18, 17, 16, 15};
  size_t block3[5] = { 9,  8,  7,  6,  5};
  size_t block4[5] = {22, 23, 24, 25, 26};
  size_t block5[5] = {32, 33, 34, 35, 36};
  size_t block_size = sizeof(block1)/sizeof(size_t); 

  BlockRMQDatabase* block_rmq_db = BRD_create(block_size);
  size_t ret = 0;
  
  ret = BRD_lookup(block_rmq_db, block1, block_size, 0, 5);
  mu_assert(ret == 0, "Failed BRD_lookup. Expected %d, got %zu", 0, ret);
  ret = BRD_lookup(block_rmq_db, block1, block_size, 1, 4);
  mu_assert(ret == 1, "Failed BRD_lookup. Expected %d, got %zu", 1, ret);
  
  ret = BRD_lookup(block_rmq_db, block2, block_size, 0, 5);
  mu_assert(ret == 4, "Failed BRD_lookup. Expected %d, got %zu", 4, ret);
  ret = BRD_lookup(block_rmq_db, block2, block_size, 1, 4);
  mu_assert(ret == 3, "Failed BRD_lookup. Expected %d, got %zu", 3, ret);

  ret = BRD_lookup(block_rmq_db, block3, block_size, 0, 5);
  mu_assert(ret == 4, "Failed BRD_lookup. Expected %d, got %zu", 4, ret);
  ret = BRD_lookup(block_rmq_db, block3, block_size, 1, 4);
  mu_assert(ret == 3, "Failed BRD_lookup. Expected %d, got %zu", 3, ret);

  ret = BRD_lookup(block_rmq_db, block4, block_size, 0, 5);
  mu_assert(ret == 0, "Failed BRD_lookup. Expected %d, got %zu", 0, ret);
  ret = BRD_lookup(block_rmq_db, block4, block_size, 1, 4);
  mu_assert(ret == 1, "Failed BRD_lookup. Expected %d, got %zu", 1, ret);
  
  ret = BRD_lookup(block_rmq_db, block5, block_size, 0, 5);
  mu_assert(ret == 0, "Failed BRD_lookup. Expected %d, got %zu", 0, ret);
  ret = BRD_lookup(block_rmq_db, block5, block_size, 1, 4);
  mu_assert(ret == 1, "Failed BRD_lookup. Expected %d, got %zu", 1, ret);

  BRD_delete(block_rmq_db);
  return NULL;
}

/* Verify the BlockRMQDatabase for a range of block sizes. */
char*  test_brd_lookup_range()
{

  int block_size = 0;
  for(block_size = 1; block_size < 8; block_size++) {
    BlockRMQDatabase* block_rmq_db = BRD_create(block_size);
    BRD_verify(block_rmq_db);
    BRD_delete(block_rmq_db);
  }

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_block_ids);
  mu_run_test(test_brt_lookup);
  mu_run_test(test_brt_lookup_random);

  mu_run_test(test_brd_lookup);
  mu_run_test(test_brd_lookup_range);

  return NULL;
}

RUN_TESTS(all_tests);
