#ifndef _euler_tour_H_
#define _euler_tour_H_

#include "suffix_tree/suffix_tree.h"

/* TYPES */

/* A struct that holds the relevant information for the Euler tour of a
 * SuffixTree_T. This includes the nodes in the order that they are visited,
 * their depths, and the first time each node appears in the tour.
 */
typedef struct EulerTour_T* EulerTour_T;

struct EulerTour_T {
  /*
   * The nodes of the SuffixTree_T in the order that they are visited in the
   * tour.
   */
  Node_T*            nodes;

  /* Index of the first instance of each node in the tour. */
  size_t*            first_instances;

  /* The depth of each node in the tour. */
  SuffixTreeIndex_T* depths;

  /*
   * The length of the tour. This is the length of the nodes and depths arrays.
   */
  size_t             length;

  /*
   * The number of unique nodes in the tour. Same as the number of nodes in the
   * associated SuffixTree_T.
   */
  size_t             num_nodes;
};

/* FUNCTIONS */

/* Create an EulerTour_T from a SuffixTree_T. */
EulerTour_T EulerTour_create(SuffixTree_T tree);

/* Delete an EulerTour_T, freeing all its memory. */
void EulerTour_delete(EulerTour_T* euler_tour);

/* 
 * Run some test on an EulerTour_T using its associated SuffixTree_T.
 *
 * Returns:
 *  0 if tests pass, else 1.
 */
int EulerTour_verify(EulerTour_T euler_tour, SuffixTree_T tree);

/* Print the EulerTour_T to stdout. */
void EulerTour_print(EulerTour_T euler_tour);

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
