#ifndef _normalized_blocks_private_H_
#define _normalized_blocks_private_H_

#include "normalized_blocks.h"

typedef struct BlockRMQTable_T* BlockRMQTable_T;

/* BlockRMQTable FUNCTIONS */
BlockRMQTable_T BlockRMQTable_create(const size_t* block, size_t block_size);
void            BlockRMQTable_delete(BlockRMQTable_T* table);
size_t          BlockRMQTable_lookup(BlockRMQTable_T table,
                                     size_t i, size_t j);
void            BlockRMQTable_print(BlockRMQTable_T table);
int             BlockRMQTable_verify(BlockRMQTable_T table,
                                     const size_t* block);


unsigned int get_block_id(const size_t* block, size_t block_size);

#endif
