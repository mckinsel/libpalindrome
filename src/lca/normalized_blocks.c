#include <stdlib.h>

#include "dbg.h"
#include "lca/normalized_blocks.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

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
