#include <math.h>
#include <stdlib.h>

#include "dbg.h"
#include "euler_tour.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

/*
 * Create the depth and first instance arrays that are used in the
 * constant-time lowest common ancestor algorithm. The depths array is the
 * depth of each node visited in an Euler tour of the suffix tree. The first
 * instances array gives the index in the depths array of the first instance of
 * a node. Depth and first instance arrays correspond to the L and R array
 * described in the paper above.
 *
 * Inputs:
 *  SUFFIX_TREE* stree        :   Suffix tree on which we want to perform LCA
 *  size_t** tour             :   Node ids as visited in the tour
 *  size_t** depths           :   Depth array
 *  size_t** first_intances   :   First instance array
 * 
 * Outputs:
 *  None, but depths and first_instances are allocated, and the caller is
 *  responsible for freeing.
 */
void euler_tour_arrays_create(const SUFFIX_TREE* stree,
                              size_t** tour,
                              size_t** depths,
                              size_t** first_instances)
{
  /* The length of the Euler tour */
  size_t array_size = 2 * stree->num_nodes - 1;
  
  *tour = calloc(array_size, sizeof(size_t));
  *depths = calloc(array_size, sizeof(size_t));
  *first_instances = calloc(stree->num_nodes, sizeof(size_t));
  
  size_t* pos_in_tour = calloc(1, sizeof(size_t));
  euler_tour(stree->root, 0, pos_in_tour, *tour, *depths, *first_instances);

  free(pos_in_tour);
}

/*
 * Complete an Euler tour starting at a node. Record node depths and the first
 * instances of nodes during the tour. This is called by prepare_rmq_arrays,
 * starting with the root of the suffix tree.
 *
 * Inputs:
 *  NODE* node                :   Starting node
 *  size_t* pos_in_tour       :   Number of nodesh visited in the tour so far
 *  size_t* tour              :   Node ids array
 *  size_t* depths            :   Depths array
 *  size_t* first_instances   :   First instances array
 *
 * Outputs:
 *  None, but populates depths and first_instances.
 */
void euler_tour(NODE* node, size_t depth, size_t* pos_in_tour,
                size_t* tour, size_t* depths, size_t* first_instances)
{
  NODE* next_node = node->sons;
  
  tour[*pos_in_tour] = node->index;
  depths[*pos_in_tour] = depth;
  if(first_instances[node->index] == 0) {
    first_instances[node->index] = *pos_in_tour;
  }
  (*pos_in_tour)++;
    
  if(next_node != 0) {
    while(next_node != 0) {
      euler_tour(next_node, depth + 1, pos_in_tour,
                 tour, depths, first_instances); 
      tour[*pos_in_tour] = node->index;
      depths[*pos_in_tour] = depth;
      (*pos_in_tour)++;
      next_node = next_node->right_sibling;
    }
    
  }

}

/*
 * Test the depth and first_instance arrays. Return 0 if tests pass, else 1.
 */
int verify_rmq_arrays(const SUFFIX_TREE* stree, const size_t* tour,
                      const size_t* depths,
                      const size_t* first_instances)
{
  /* First verify that the depths array has the +-1 property */
  size_t i = 0;
  for(i = 0; i < 2 * stree->num_nodes - 2; i++) { /* Note - 2 not - 1 */
    long int diff = depths[i] - depths[i + 1];
    
    if(!labs(diff) == 1) {
      log_warn("Consecutive values in depth array do not differ by 1.");
      return 1;
    }
  }

  /* Then check some properties of the tour. The first and last elements should
   * be the root. And, for A, B, C in the tour, if A == C, then B is a leaf in
   * the tree, and the number of leaves is the lenght of the suffix tree
   * string. */
  if(tour[0] != stree->root->index) {
    log_warn("First element in tour is not the root.");
    return 1;
  }
  if(tour[2 * stree->num_nodes - 2] != stree->root->index) {
    log_warn("Last element in tour is not the root.");
    return 1;
  }
  size_t leaf_count = 0;
  for(i = 0; i < 2 * stree->num_nodes - 3; i++) {
    if(tour[i] == tour[i+2]) leaf_count++;
  }
  if(leaf_count != stree->length) {
    log_warn("Number of leaves in tour, %zu, is not the length of the string, %zu",
            leaf_count, stree->length);
    return 1;
  }

  /* Verify that first instances is strictly increasing. This doesn't
   * necessarily have to be true, but since we assign labels in the order that
   * we visit nodes in the same depth-first traversal, it should be true for
   * us. */
  for(i = 0; i < stree->num_nodes - 1; i++) {
    if(first_instances[i+1] <= first_instances[i]){
      log_warn("First instances is not increasing.");
      return 1;
    }
  }

  return 0;
}

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
size_t get_block_index(size_t pos, size_t n)
{
  return pos / get_block_size(n);
}

size_t get_pos_in_block(size_t pos, size_t n)
{
  return pos % get_block_size(n);
}

size_t get_block_size(size_t n)
{
  return (size_t)ceil(log2(n)/2);
}

size_t get_num_blocks(size_t n)
{
  return (size_t)ceil((double)n/get_block_size(n));
}

/* 
 * Get the values of the array in a block.
 * 
 * Inputs:
 *    size_t** block        :   The block whose values will be populated
 *    size_t  block_index   :   Index of the desired block
 *    size_t* array         :   array from which the block comes
 *    size_t array_size     :   size of array
 *
 * Outputs:
 *    Size of the block. This can be less than the expected block_size if it's
 *    the final block.
 */
size_t get_block(size_t** block, size_t block_index,
                 const size_t* array, size_t array_size)
{
  size_t block_size = get_block_size(array_size);
  size_t block_start = block_size * block_index;
  size_t block_end = MIN(array_size, block_start + block_size);
  size_t actual_block_size = block_end - block_start;  
  memcpy(*block, array + block_start, actual_block_size*sizeof(size_t));
  
  return actual_block_size;
}


/*
 * Calculate the minimum over each block in the depth array. Keep track of both
 * the value of the minimum and its position in the block.
 */
size_t get_block_minima(const size_t* depths, size_t depths_size,
                        size_t** block_minima, size_t** minima_positions)
{
  size_t num_blocks = get_num_blocks(depths_size);
  *block_minima = calloc(num_blocks, sizeof(size_t));
  check_mem(*block_minima);
  *minima_positions = calloc(num_blocks, sizeof(size_t));
  check_mem(*minima_positions);
  
  size_t current_block = 0;
  size_t pos_in_current_block = 0;
  size_t minimum_pos_in_current_block = (size_t)-1;
  size_t current_minimum = (size_t)-1; 
  size_t block = 0;

  size_t i = 0;
  for(i = 0; i < depths_size; i++) {

    block = get_block_index(i, depths_size);
    /* First check and see if this is a new block. If so, record the minimum
     * and position of the last block and reset those values. */
    if(block != current_block) {
      (*block_minima)[current_block] = current_minimum;
      (*minima_positions)[current_block] = minimum_pos_in_current_block;
      current_minimum = (size_t)-1;
      minimum_pos_in_current_block = (size_t)-1;
      pos_in_current_block = 0;
      current_block = block;
    }

    /* Then see if the depths array value at this position is lower than the
     * current minimum for this block. */
    if(depths[i] < current_minimum) {
      current_minimum = depths[i];
      minimum_pos_in_current_block = pos_in_current_block;
    }
    pos_in_current_block++;
  }
  (*block_minima)[block] = current_minimum;
  (*minima_positions)[block] = minimum_pos_in_current_block;
  
  return num_blocks;

error:
  if(*block_minima) free(*block_minima);
  if(*minima_positions) free(*minima_positions);
  return 0;
}
