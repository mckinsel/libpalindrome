#ifndef _normalized_blocks_H_
#define _normalized_blocks_H_

/* Struct and functions for performing RMQ on a single block */
typedef struct {
  size_t** table;
  size_t   block_size;
} BlockRMQTable;

BlockRMQTable* BRT_create(const size_t* block, size_t block_size);
void BRT_delete(BlockRMQTable* block_rmq_table);
size_t BRT_lookup(const BlockRMQTable* block_rmq_table,
                  size_t i, size_t j);
void BRT_print(const BlockRMQTable* block_rmq_table);
int BRT_verify(const BlockRMQTable* block_rmq_table, const size_t* block);


/* Struct and functions for looking up and creating BlockRMQTables */
typedef struct {
  size_t            block_size;
  unsigned int      num_blocks;
  BlockRMQTable**   block_tables;
  int*              is_initialized;
  
  BlockRMQTable*    remainder_block_table;
  size_t            remainder_block_id;
  int               remainder_is_initialized;
} BlockRMQDatabase;

BlockRMQDatabase* BRD_create(size_t block_size);
size_t BRD_lookup(BlockRMQDatabase* block_rmq_db, const size_t* block,
                  size_t block_size, size_t i, size_t j);
void BRD_delete(BlockRMQDatabase* block_rmq_db);
int BRD_verify(BlockRMQDatabase* block_rmq_db);

unsigned int get_block_id(const size_t* block, size_t block_size);

#endif
