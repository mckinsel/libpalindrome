#ifndef _normalized_blocks_H_
#define _normalized_blocks_H_

/* RMQ means range minimum query. */

/* TYPES */
typedef struct BlockRMQDatabase_T* BlockRMQDatabase_T;

/* BlockRMQDatabase FUNCTIONS */
BlockRMQDatabase_T BlockRMQDatabase_create(size_t block_size);
void               BlockRMQDatabase_delete(BlockRMQDatabase_T* block_rmq_db);
size_t             BlockRMQDatabase_lookup(BlockRMQDatabase_T block_rmq_db,
                                           const size_t* block,
                                           size_t block_size,
                                           size_t i, size_t j);
int                BlockRMQDatabase_verify(BlockRMQDatabase_T block_rmq_db);

#endif
