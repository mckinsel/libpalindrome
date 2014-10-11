/* Defines functions related to the Euler tour of the suffix tree. */
#ifndef _euler_tour_H_
#define _euler_tour_H_

#include "suffix_tree/suffix_tree.h"

/* 
 * Perform the Euler tour of the suffix tree, recording each node in tour,
 * the depth of each node in depths and the first time a node appears in
 * depths in first_instances.
 */
void euler_tour_arrays_create(const SUFFIX_TREE* stree,
                              size_t** tour,
                              size_t** depths,
                              size_t** first_instances);

/* 
 * Perform the Euler tour starting at node. Helper for
 * euler_tour_arrays_create, and proceeds recursively through node's subtree.
 */
void euler_tour(NODE* node, size_t depth, size_t* pos_in_tour,
                size_t* tour, size_t* depths, size_t* first_instances);

/* Test that the depths and first_instances arrays are correct. */
int verify_rmq_arrays(const SUFFIX_TREE* stree,
                      const size_t* tour,
                      const size_t* depths,
                      const size_t* first_instances);

/* 
 * Functions for partitioning the depth array into blocks. This is a key step
 * in the constant-time LCA algorithm, and the size of the blocks needs to be
 * log(n)/2.
 */

/* Size of the blocks if the full array is of size n. */
size_t get_block_size(size_t n);

/* 
 * Number of blocks in array of size n, including possibly irregularly sized
 * final block.
 */
size_t get_num_blocks(size_t n);

/* Index of the block position pos in the array falls. */
size_t get_block_index(size_t pos, size_t n);

/* The position of pos in its block. */
size_t get_pos_in_block(size_t pos, size_t n);

/* Get the block itself, returning the block size. */
size_t get_block(size_t** block, size_t block_index,
                 const size_t* array, size_t array_size);

/* Function to get block minima, the A` and B arrays from the paper. */
size_t get_block_minima(const size_t* depths, size_t depths_size,
                        size_t** block_minima, size_t** minima_positions);
#endif
