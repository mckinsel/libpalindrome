#include "tour_partition.h"

#include "dbg.h"

#include <math.h>

#define MIN(a,b) ((a) < (b) ? a : b)

/*
 * Functions for partitioning the depth array. Per the LCA algorithm, it needs
 * to be partitioned into blocks of size log(n)/2.
 *
 * get_block_index returns block index for a given position and a given
 * total array size.
 *
 * get_pos_in_block returns the position within a block of a position in the
 * array.
 *
 * get_block_size returns the size of all but possibly the final block of
 * the array. The last block is the remainder so can be irregularly sized.
 *
 * get_num_blocks returns the number of blocks.
 */
size_t get_block_size(size_t n)
{
  if(n == 1) return 1;
  return (size_t)ceil(log2(n)/2);
}

size_t get_block_index(size_t pos, size_t n)
{
  return pos / get_block_size(n);
}

size_t get_pos_in_block(size_t pos, size_t n)
{
  return pos % get_block_size(n);
}

size_t get_num_blocks(size_t n)
{
  return (size_t)ceil((double)n/get_block_size(n));
}

/* Create a partition. */
TourPartition_T TourPartition_create(const size_t* values, size_t values_length)
{
  if(values_length == 0) {
    log_warn("Cannot partition an empty array of values.");
    return NULL;
  }

  TourPartition_T tour_partition = calloc(1, sizeof(struct TourPartition_T));
  check_mem(tour_partition);
  tour_partition->num_blocks = get_num_blocks(values_length);
  tour_partition->block_length = get_block_size(values_length);
  tour_partition->total_length = values_length;
  
  tour_partition->block_minima = calloc(tour_partition->num_blocks,
                                       sizeof(size_t));
  check_mem(tour_partition->block_minima);

  tour_partition->minima_positions = calloc(tour_partition->num_blocks,
                                           sizeof(size_t));
  check_mem(tour_partition->minima_positions);
  
  size_t current_block = 0;
  size_t pos_in_current_block = 0;
  size_t minimum_pos_in_current_block = (size_t)-1;
  size_t current_minimum = (size_t)-1; 
  size_t block = 0;

  size_t i = 0;
  for(i = 0; i < values_length; i++) {

    block = get_block_index(i, values_length);
    /* First check and see if this is a new block. If so, record the minimum
     * and position of the last block and reset those values. */
    if(block != current_block) {
      tour_partition->block_minima[current_block] = current_minimum;
      tour_partition->minima_positions[current_block] = minimum_pos_in_current_block;
      current_minimum = (size_t)-1;
      minimum_pos_in_current_block = (size_t)-1;
      pos_in_current_block = 0;
      current_block = block;
    }

    /* Then see if the depths array value at this position is lower than the
     * current minimum for this block. */
    if(values[i] < current_minimum) {
      current_minimum = values[i];
      minimum_pos_in_current_block = pos_in_current_block;
    }
    pos_in_current_block++;
  }
  tour_partition->block_minima[block] = current_minimum;
  tour_partition->minima_positions[block] = minimum_pos_in_current_block;
  
  return tour_partition;

error:
  TourPartition_delete(&tour_partition);
  return NULL;
}

void TourPartition_delete(TourPartition_T* tour_partition)
{
  if(*tour_partition) {
    if((*tour_partition)->block_minima) free((*tour_partition)->block_minima);
    if((*tour_partition)->minima_positions) free((*tour_partition)->minima_positions);
    free(*tour_partition);
  }
}

size_t TourPartition_get_block(size_t** block, TourPartition_T tour_partition,
                               size_t block_index, const size_t* values)
{
  
  size_t block_start = tour_partition->block_length * block_index;
  size_t block_end = MIN(tour_partition->total_length,
                         block_start + tour_partition->block_length);
  size_t actual_block_size = block_end - block_start;  
  memcpy(*block, values + block_start, actual_block_size * sizeof(size_t));
  
  return actual_block_size;
}

size_t TourPartition_get_block_index(TourPartition_T tour_partition,
                                     size_t position)
{
  return get_block_index(position, tour_partition->total_length);
}
  
size_t TourPartition_get_pos_in_block(TourPartition_T tour_partition,
                                      size_t position)
{
  return get_pos_in_block(position, tour_partition->total_length);
}

int TourPartition_verify(TourPartition_T tour_partition, const size_t* values,
                         size_t values_length)
{
  size_t* block_counts = calloc(tour_partition->num_blocks, sizeof(size_t));
  
  size_t i = 0;
  size_t last_block = 0;
  for(i = 0; i < values_length; i++) {
    size_t block = TourPartition_get_block_index(tour_partition, i);
    if(!(block == last_block || block == last_block + 1)) {
      log_warn("Block assignments are not increasing.");
      return 1;
    }
    last_block = block;
    block_counts[block]++;
  }
  
  size_t running_position_sum = 0;
  for(i = 0; i < tour_partition->num_blocks; i++) {
    running_position_sum += block_counts[i];

    if(i != tour_partition->num_blocks - 1) {
      if(block_counts[i] != tour_partition->block_length) {
        free(block_counts);
        log_warn("Non-final block has an incorrect length.");
        return 1;
      }
    } else {
      if(block_counts[i] == 0) {
        free(block_counts);
        log_warn("Empty final block.");
        return 1;
      }
    }
  }

  if(running_position_sum != values_length) {
    free(block_counts);
    log_warn("Total block assignments does not equal size of input array.");
    return 1;
  }

  free(block_counts);
  
  size_t* block = malloc(tour_partition->block_length * sizeof(size_t));
  for(i = 0; i < tour_partition->num_blocks; i++) {
    size_t block_index = TourPartition_get_block_index(tour_partition, i);
    size_t block_length = TourPartition_get_block(&block, tour_partition,
                                                  block_index, values);
    size_t j = 0;
    size_t min_value = (size_t)-1;
    size_t min_pos = (size_t)-1;
    for(j = 0; j < block_length; j++) {
      if(block[j] < min_value) {
        min_value = block[j];
        min_pos = j;
      }
    }
    if(min_pos != tour_partition->minima_positions[i]) {
      log_warn("Incorrect minimum position in block %zu.", i);
      free(block);
      return 1;
    }
  }
  free(block);

  return 0;
}
