/*
 * This is an implentation of constant-time lowest common ancestor queries for
 * nodes in a suffix tree, as defined in ../suffix_tree/suffix_tree.h
 *
 * There method used is described in "The LCA Problem Revisited" by Michael A.
 * Bender and Martin Fararch-Colton.
 *
 * There are three main steps:
 *    1. Create an array on which we will perform range minimum queries. This
 *    array is the depths of nodes visited in an Euler tour of the tree.
 *    2. Partition that array into blocks, and compute minima between blocks
 *    using the Sparse Table algorithm. This creates and retains a table of size
 *    nlogn.
 *    3. Precompute all possible within-block queries. This creates and stores
 *    tables of total size sqrt(n)log^2n.
 */


#include <math.h>
#include <stdlib.h>

#include "dbg.h"
#include "suffix_tree/suffix_tree.h"

#include "lca.h"

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
 *  DBL_WORD** depths         :   Depth array
 *  DBL_WORD** first_intances :   First instance array
 * 
 * Outputs:
 *  None, but depths and first_instances are allocated, and the caller is
 *  responsible for freeing.
 */
void prepare_rmq_arrays(const SUFFIX_TREE* stree, DBL_WORD** depths,
                        DBL_WORD** first_instances)
{
  /* The length of the Euler tour */
  DBL_WORD array_size = 2 * stree->num_nodes - 1;

  *depths = calloc(array_size, sizeof(DBL_WORD));
  *first_instances = calloc(stree->num_nodes, sizeof(DBL_WORD));
  
  DBL_WORD* pos_in_tour = calloc(1, sizeof(DBL_WORD));
  euler_tour(stree->root, 0, pos_in_tour, *depths, *first_instances);

  free(pos_in_tour);
}

/*
 * Complete an Euler tour starting at a node. Record node depths and the first
 * instances of nodes during the tour. This is called by prepare_rmq_arrays,
 * starting with the root of the suffix tree.
 *
 * Inputs:
 *  NODE* node                :     Starting node
 *  DBL_WORD* pos_in_tour     :   Number of nodes visited in the tour so far
 *  DBL_WORD* depths          :   Depths array
 *  DBL_WORD* first_instances :   First instances array
 *
 * Outputs:
 *  None, but populates depths and first_instances.
 */
void euler_tour(NODE* node, DBL_WORD depth, DBL_WORD* pos_in_tour,
                DBL_WORD* depths, DBL_WORD* first_instances)
{
  NODE* next_node = node->sons;
  
  depths[*pos_in_tour] = depth;
  if(first_instances[node->index] == 0) {
    first_instances[node->index] = *pos_in_tour;
  }
  (*pos_in_tour)++;
    
  if(next_node != 0) {
    while(next_node != 0) {
      euler_tour(next_node, depth + 1, pos_in_tour,
                 depths, first_instances); 
      depths[*pos_in_tour] = depth;
      (*pos_in_tour)++;
      next_node = next_node->right_sibling;
    }
    
  }

}

/*
 * Test the depth and first_instance arrays. Return 0 if tests pass, else 1.
 */
int verify_rmq_arrays(const SUFFIX_TREE* stree, const DBL_WORD* depths,
                      const DBL_WORD* first_instances)
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
 * get_parition returns block index for a given position and a given total
 * array size.
 *
 * get_partition_size returns the size of all but possibly the final block of
 * the array. The last block is the remained so can be irregularly sized.
 *
 * get_num_partition returns the number of blocks.
 */
size_t get_partition(size_t pos, size_t n)
{
  return pos / get_partition_size(n);
}
size_t get_partition_size(size_t n)
{
  return (size_t)ceil(log2(n)/2);
}
size_t get_num_partitions(size_t n)
{
  return (size_t)ceil((double)n/get_partition_size(n));
}

/*
 * Calculate the minimum over each block in the depth array. Keep track of both
 * the value of the minimum and its position in the block.
 */
void get_partition_minima(const size_t* depths, size_t depths_size,
                          size_t** block_minima, size_t** minima_positions)
{
  *block_minima = calloc(get_num_partitions(depths_size), sizeof(size_t));
  *minima_positions = calloc(get_num_partitions(depths_size), sizeof(size_t));
  
  size_t current_block = 0;
  size_t pos_in_current_block = 0;
  size_t minimum_pos_in_current_block = (size_t)-1;
  size_t current_minimum = (size_t)-1; 
  size_t block = 0;

  size_t i = 0;
  for(i = 0; i < depths_size; i++) {

    block = get_partition(i, depths_size);
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

}
                      

/*
 * Create an array that maps position in a string to a leaf of a suffix tree.
 * It's important to remember that the string associated with a leaf has an
 * additional '$' char at the end that wasn't in the original string.
 *
 * Inputs:
 *    SUFFIX_TREE* stree    :   suffix tree build from string
 *    size_t str_len        :   length of the string
 *
 * Outputs:
 *    NODE** leaf_map   :   an array such that leaf_map[i] is a pointer to the
 *                          leaf of stree for the suffix starting at position
 *                          i in the string
 * 
 * The caller is responsible for freeing leaf_map.
 */
NODE** map_position_to_leaf(const SUFFIX_TREE* stree, size_t str_len)
{
  NODE** leaf_map = calloc(str_len, sizeof(NODE*));
  check_mem(leaf_map);

  NODE* child = stree->root->sons;
  while(child != 0) {
    map_position_to_leaf_dfs(stree, child, leaf_map, 0);
    child = child->right_sibling;
  }
  return leaf_map;

error:
  if(leaf_map) free(leaf_map);
  return NULL;
}

/*
 * A helper function for map_position_to_leaf that traverses the suffix tree
 * depth first and writes values into leaf_map.
 *
 * Inputs:
 *  SUFFIX_TREE* stree    :   Suffix tree of the string
 *  NODE* node            :   Current node in stree in the DFS
 *  NODE** leaf_map       :   The array of string indices to leaf pointers
 *  size_t prev_suf_length:   The length of suffixes from node to the root
 *
 * Outputs:
 *  None, but modifies leaf_map;
 */
void map_position_to_leaf_dfs(const SUFFIX_TREE* stree,
                              NODE* node,
                              NODE** leaf_map,
                              size_t prev_suf_length)
{
  size_t edge_start = node->edge_label_start;
  size_t edge_end = get_node_label_end(stree, node);
  size_t current_suf_length = prev_suf_length + edge_end - edge_start + 1;
  
  if(edge_end == stree->e) {
    size_t suffix_start = stree->e - current_suf_length;
    if(suffix_start + 1 != stree->e) {
      leaf_map[suffix_start] = node;
    }
    return;
  }

  NODE* next_node = node->sons;
  while(next_node != 0) {
    map_position_to_leaf_dfs(stree, next_node, leaf_map,
                             current_suf_length);
    next_node = next_node->right_sibling;
  }
}

/*
 * Test the the pos_to_leaf map is correct by walking up the tree from the
 * leaves.
 *
 * Inputs:
 *    NODE** pos_to_leaf    :   Array from string position to stree leaf
 *    SUFFIX_TREE* stree    :   The suffix tree
 *    size_t query_len      :   Length of the string
 *
 * Outputs:
 *    0 if pos_to_leaf is correct, otherwise 1
 */
int verify_map_position_to_leaf(NODE** pos_to_leaf,
                                const SUFFIX_TREE* stree,
                                size_t query_len)
{
  size_t i = 0;
  size_t suffix_depth = 0;
  size_t incoming_start = 0;
  size_t incoming_end = 0;

  for(i = 0; i < query_len; i++) {

    const NODE* node = pos_to_leaf[i];
    suffix_depth = 0;
    while(node->father != 0) {
      incoming_start = node->edge_label_start;
      incoming_end = get_node_label_end(stree, node);
      /* Stop when we reach the edge to the root */
      suffix_depth += incoming_end - incoming_start + 1;
      node = node->father;
    }

    if(i != query_len - (suffix_depth - 1)) {
      log_warn("Leaf at position %zd has suffix of length %zd.", i, suffix_depth);
      return 1;
    }
  }
  return 0;
}


