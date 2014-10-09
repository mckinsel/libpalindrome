#include <stdlib.h>

#include "dbg.h"
#include "lca/normalized_blocks.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

/* 
 * Create a BlockRMQTable that can perform a range minimum query on a block in
 * nlogn time.
 * 
 * Input:
 *  size_t* block       :     Block for which RMQ table will be created
 *  size_t block_size   :     Size of block
 * 
 * Output:
 *  BlockRMQTable*, the created struct.
 */
BlockRMQTable* BRT_create(const size_t* block, size_t block_size)
{
  BlockRMQTable* block_rmq_table = malloc(sizeof(BlockRMQTable));
  check_mem(block_rmq_table);
  block_rmq_table->block_size = block_size;
  
  /* Allocate the lookup table */
  block_rmq_table->table = calloc(block_size, sizeof(size_t*));
  check_mem(block_rmq_table->table);
  size_t i = 0;
  for(i = 0; i < block_size; i++) {
    block_rmq_table->table[i] = calloc(block_size - i, sizeof(size_t));
    check_mem(block_rmq_table->table[i]);
  }
  
  /* Populate the table using dynamic programming */ 
  size_t j = 0;
  for(i = 0; i < block_size; i++) {
    for(j = 0; j < block_size - i; j++) {
      size_t j_pos_in_block = j + i + 1;
      
      /* The min pos in [i:i+1] is just i */
      if(j_pos_in_block - i == 1) {
        block_rmq_table->table[i][j] = i;
        continue;
      }
      
      /* Otherwise we just compare [i:j-i] and j */
      size_t prev_min_val = block[block_rmq_table->table[i][j-1]];
      size_t curr_min_val = block[j_pos_in_block - 1];
      if(prev_min_val <= curr_min_val) {
        block_rmq_table->table[i][j] = block_rmq_table->table[i][j-1];
      } else {
        block_rmq_table->table[i][j] = j_pos_in_block - 1;
      }
    }
  }
  return block_rmq_table;

error:
  BRT_delete(block_rmq_table);
  return NULL;
}

/* Free a BlockRMQTable. */
void BRT_delete(BlockRMQTable* block_rmq_table)
{
  if(block_rmq_table) {
    if(block_rmq_table->table) {
      size_t i = 0;
      for(i = 0; i < block_rmq_table->block_size; i++) {
        if(block_rmq_table->table[i]) free(block_rmq_table->table[i]);
      }
      free(block_rmq_table->table);
    }
    free(block_rmq_table);
  }
}

/* 
 * Find the position of the minimum element in a block. The block itself isn't
 * needed because we've already created the lookup table for the block.
 *
 * Input:
 *  BlockRMQTable* block_rmq_table    :  Table for the block
 *  size_t i                          :  Start pos for RMQ
 *  size_t j                          :  End pos for RMQ
 *  
 * Output:
 *  size_t, position of minimum element  in [i:j] in the block.
 */ 
size_t BRT_lookup(const BlockRMQTable* block_rmq_table,
                  size_t i, size_t j)
{
  check(j != i, "Cannot find maximum in empty range [%zu:%zu]", i, j);
  
  size_t start = MIN(i, j);
  size_t end = MAX(i, j);
  
  check(end <= block_rmq_table->block_size,
        "Cannot look past end of block for BlockRMQTable lookup. Block size "
        "is %zu, but j is %zu", block_rmq_table->block_size, j);
  
  size_t end_pos_in_table = end - start - 1;
  return block_rmq_table->table[start][end_pos_in_table];

error:
  return (size_t)-1;
}

/* Print a BlockRMQTable. */
void BRT_print(const BlockRMQTable* block_rmq_table)
{
  printf("BlockRMQTable for block of size %zu:\n", block_rmq_table->block_size);

  size_t i = 0;
  size_t j = 0;
  for(i = 0; i < block_rmq_table->block_size; i++) {
    for(j = 0; j < i; j++) {
      printf("    ");
    }
    for(j = 0; j < block_rmq_table->block_size -  i - 1; j++) {
      size_t t_val = block_rmq_table->table[i][j];
      if(t_val < 10) {
        printf("   %zu", t_val);
      } else if(t_val < 100) {
        printf("  %zu", t_val);
      } else {
        printf(" %zu", t_val);
      }
    }
    printf("\n");
  }
}

/* 
 * Test a BlockRMQTable by checking its returned minimum elements against the
 * minimum elements calculated by checking all values in the interval.
 */
int BRT_verify(const BlockRMQTable* block_rmq_table, const size_t* block)
{
  size_t i = 0;
  size_t j = 0;
  
  for(i = 0; i <= block_rmq_table->block_size; i++) {
    for(j = 0; j <= block_rmq_table->block_size; j++) {
      /* j == i won't work */
      if(j == i) continue;

      /* Find the minimum by iterating over the whole range. */
      size_t min_val = (size_t)-1;
      size_t min_pos = 0;

      size_t start = MIN(i, j);
      size_t end = MAX(i, j);
      size_t pos = 0;
      for(pos = start; pos < end; pos++) {
        if(block[pos] <= min_val) {
          min_val = block[pos];
          min_pos = pos;
        }
      }
      
      size_t test_pos = BRT_lookup(block_rmq_table, i, j);
      if(test_pos != min_pos) {
        log_warn("Min element position in [%zu:%zu] should be %zu, but "
                 "BRT_lookup returned %zu.", i, j, min_pos, test_pos);
        return 1;
      }
    }
  }

  return 0;
}

/* 
 * Create a BlockRMQDatabase struct that can be used to perform RMQs on +-1
 * blocks of a given block_size.
 */
BlockRMQDatabase* BRD_create(size_t block_size)
{
  
  BlockRMQDatabase* block_rmq_db = malloc(sizeof(BlockRMQDatabase));
  check_mem(block_rmq_db);

  block_rmq_db->block_size = block_size;
  check(block_size > 0, "Cannot create BlockRMQDatabase for empty blocks.");
  block_rmq_db->num_blocks = 1 << (block_size - 1);
  
  block_rmq_db->is_initialized = calloc(block_rmq_db->num_blocks, sizeof(int));
  check_mem(block_rmq_db->is_initialized);

  block_rmq_db->block_tables = calloc(block_rmq_db->num_blocks, sizeof(BlockRMQTable*));
  check_mem(block_rmq_db->block_tables);
  
  return block_rmq_db;

error:
  BRD_delete(block_rmq_db);
  return NULL;
}

/* 
 * Perform a range minimum query using the BlockRMQDatabase.
 *
 * Input:
 *  BlockRMQDatabase* block_rmq_db  :   DB containing RMQ tables
 *  size_t* block                   :   Block on which RMQ is performed
 *  size_t block_size               :   Size of block
 *  size_t i                        :   Start position for RMQ
 *  size_t j                        :   End position for RMQ
 *
 * Output:
 *  size_t, position of minimum element in [i:j] in block.
 */
size_t BRD_lookup(const BlockRMQDatabase* block_rmq_db, const size_t* block,
                  size_t block_size, size_t i, size_t j)
{
  check(block_size == block_rmq_db->block_size,
        "Block size %zu is different than the DB block size %zu.",
        block_size,  block_rmq_db->block_size);

  unsigned int block_id = get_block_id(block, block_size);
  
  /* Create the block table if it doesn't exist yet. */
  if(!block_rmq_db->is_initialized[block_id]) {
    check(!block_rmq_db->block_tables[block_id],
          "Attempting to initialize block %u twice.",
          block_id);

    block_rmq_db->block_tables[block_id] = BRT_create(block, block_size);
    check(block_rmq_db->block_tables[block_id], "Block table creation failed.");
    block_rmq_db->is_initialized[block_id] = 1;
  }
  
  BlockRMQTable* block_rmq_table = block_rmq_db->block_tables[block_id];
  check(block_rmq_table, "BlockRMQTable not initialized.");

  return BRT_lookup(block_rmq_table, i, j);

error:
  return (size_t)-1;
}

/* Free a BlockRMQDatabase. */
void BRD_delete(BlockRMQDatabase* block_rmq_db)
{
  if(block_rmq_db) {
    if(block_rmq_db->is_initialized) {
      if(block_rmq_db->block_tables) {
        size_t i;
        for(i = 0; i < block_rmq_db->num_blocks; i++) {
          if(block_rmq_db->is_initialized[i] &&
             block_rmq_db->block_tables[i]) {

            BRT_delete(block_rmq_db->block_tables[i]);
          }
        }
        free(block_rmq_db->block_tables);
      }
      free(block_rmq_db->is_initialized);
    }
    free(block_rmq_db);
  }
}
      
/* Test BlockRMQDatabase lookups in a given DB. */
int BRD_verify(const BlockRMQDatabase* block_rmq_db)
{

  unsigned int i = 0;
  for(i = 0; i < block_rmq_db->num_blocks; i++) {

    size_t block[block_rmq_db->block_size];

    /* Initialize the block values so we're sure we won't end up <0. */
    size_t j = 0;
    for(j = 0; j < block_rmq_db->block_size; j++) {
      block[j] = block_rmq_db->block_size + 2;
    }
    
    /* Create a block that corresponds to the block_id given by j. */
    for(j = 0; j < block_rmq_db->block_size - 1; j++) {
      if(j | (i << (block_rmq_db->block_size - 2 - j))) {
        block[j+1] = block[j] + 1;
      } else {
        block[j+1] = block[j] - 1;
      }
    }
    
    /* Do the lookup twice. The first might initialize, the second is
     * guaranteed not to. Want to check both. */
    size_t ret = BRD_lookup(block_rmq_db, block, block_rmq_db->block_size,
                            0, block_rmq_db->block_size);
    if(ret == (size_t)-1) {
      log_warn("BRD_lookup failed.");
      return 1;
    }
    ret = BRD_lookup(block_rmq_db, block, block_rmq_db->block_size,
                     0, block_rmq_db->block_size);
    if(ret == (size_t)-1) {
      log_warn("BRD_lookup failed.");
      return 1;
    }
  }
  return 0;
}

/* 
 * Get an index for a block. Every block has the +-1 property, meaning that
 * each abs(B[j+i] - B[j]) = 1 for each j. Get an index by assigning a 1 for
 * each +1 and a 0 for each -1 in the index binary represenation. For example
 *    
 * Block:   4 5 4 3 4 5
 * Index:    1 0 0 1 1   = 19
 *
 * Input:
 *    size_t* block     :     Block getting an id
 *    size_t block_size :     Size of block
 * 
 * Output:
 *  unsigned int of the block id.
 */
unsigned int get_block_id(const size_t* block, size_t block_size)
{
  size_t i = 0;
  unsigned int block_id = 0;
  for(i = 0; i < block_size - 1; i++) {
    if(block[i+1] > block[i]){
      block_id |= (1 << (block_size - i  - 2));
    }
  }
  return block_id;
}
