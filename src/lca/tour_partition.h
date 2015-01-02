#ifndef _tour_partition_H_
#define _tour_partition_H_

#include <stdlib.h>

/* TYPES */
typedef struct TourPartition_T* TourPartition_T;

struct TourPartition_T {
  size_t  num_blocks;
  size_t  block_length;
  size_t  total_length;

  size_t* block_minima;
  size_t* minima_positions;
};

/* FUNCTIONS */
TourPartition_T TourPartition_create(const size_t* values, size_t values_length);

void            TourPartition_delete(TourPartition_T* tour_partition);

size_t          TourPartition_get_block_index(TourPartition_T tour_partition,
                                             size_t position);

size_t          TourPartition_get_pos_in_block(TourPartition_T tour_partition,
                                               size_t position);

size_t          TourPartition_get_block(size_t** block,
                                        TourPartition_T tour_partition,
                                        size_t block_index,
                                        const size_t* values);

int             TourPartition_verify(TourPartition_T tour_partition, const size_t* values,
                                     size_t values_length);
#endif
