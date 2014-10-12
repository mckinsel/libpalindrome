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

#include "lca.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

TreeLCA* TreeLCA_create(const SUFFIX_TREE* stree)
{
  TreeLCA* tree_lca = malloc(sizeof(TreeLCA));
  check_mem(tree_lca);
  
  /* First create the Euler tour */
  euler_tour_arrays_create(stree, &tree_lca->euler_tour_nodes,
                           &tree_lca->euler_tour_depths,
                           &tree_lca->node_id_pos_in_tour);
  check(tree_lca->euler_tour_nodes,
        "Failed initialization of Euler tour nodes.");
  check(tree_lca->euler_tour_depths,
        "Failed initialization of Euler tour depths.");
  check(tree_lca->node_id_pos_in_tour,
        "Failed initializtion of Euler tour first instances.")
  tree_lca->euler_tour_length = 2 * stree->num_nodes - 1;

  /* Then create the arrays for block minima. */
  tree_lca->num_blocks = get_block_minima(
      tree_lca->euler_tour_depths, tree_lca->euler_tour_length,
      &tree_lca->block_minima, &tree_lca->minima_positions);
  check(tree_lca->block_minima, "Failed block minimum array creation.");
  check(tree_lca->minima_positions, "Failed minimum position array creation.");

  /* Then create the sparse table for finding minima between blocks. */
  tree_lca->block_sparse_table = SparseTable_create(tree_lca->block_minima,
                                                    tree_lca->num_blocks);
  
  /* Finally, create the RMQ database for the blocks. */
  tree_lca->block_rmq_db = BRD_create(get_block_size(tree_lca->euler_tour_length));

  return tree_lca;

error:
  TreeLCA_delete(tree_lca);
  return NULL;
}

void TreeLCA_delete(TreeLCA* tree_lca)
{
 if(tree_lca) {
   if(tree_lca->euler_tour_nodes) free(tree_lca->euler_tour_nodes);
   if(tree_lca->euler_tour_depths) free(tree_lca->euler_tour_depths);
   if(tree_lca->node_id_pos_in_tour) free(tree_lca->node_id_pos_in_tour);

   if(tree_lca->block_minima) free(tree_lca->block_minima);
   if(tree_lca->minima_positions) free(tree_lca->minima_positions);
  
   SparseTable_delete(tree_lca->block_sparse_table);
   BRD_delete(tree_lca->block_rmq_db);
   
   free(tree_lca);
 }
}

size_t TreeLCA_lookup(const TreeLCA* tree_lca, size_t node_id1, size_t node_id2)
{
  /* First, we find a position of each requested node in the Euler tour arrays. */
  size_t tour_pos_1 = tree_lca->node_id_pos_in_tour[node_id1];
  size_t tour_pos_2 = tree_lca->node_id_pos_in_tour[node_id2];

  /* We'll need to know which one is first for some of the lookups to work
   * right. */
  size_t start_tour_pos = MIN(tour_pos_1, tour_pos_2);
  size_t end_tour_pos = MAX(tour_pos_1, tour_pos_2);

  /* Now, we want to know which block each node position falls in. */
  size_t block_index_1 = get_block_index(start_tour_pos, tree_lca->euler_tour_length);
  size_t block_index_2 = get_block_index(end_tour_pos, tree_lca->euler_tour_length);

  /* And the position, within its block, of each node position. */
  size_t pos_in_block_1 = get_pos_in_block(start_tour_pos, tree_lca->euler_tour_length);
  size_t pos_in_block_2 = get_pos_in_block(end_tour_pos, tree_lca->euler_tour_length);
  
  /* Then get the blocks themselves. Note that these are block in the node
   * depths, not the node ids. block_size_[12] give the true size of each
   * block, which may be different if it's the last block. */
  size_t* block_1 = calloc(get_block_size(tree_lca->euler_tour_length),
                           sizeof(size_t));
  size_t block_size_1 = get_block(&block_1, block_index_1, tree_lca->euler_tour_depths,
                                  tree_lca->euler_tour_length);

  size_t* block_2 = calloc(get_block_size(tree_lca->euler_tour_length),
                           sizeof(size_t));
  size_t block_size_2 = get_block(&block_2, block_index_2, tree_lca->euler_tour_depths,
                                  tree_lca->euler_tour_length);
  
  /* The value we want to calculate is the position of the minimum value
   * between the node positions in the depth array. */ 
  size_t pos_of_min_depth = 0;

  /* If the two nodes lie in the same block, then we just need to do an RMQ on
   * that block using the positions of each node within the block. */
  if(block_index_1 == block_index_2) {
    size_t min_pos_in_block = BRD_lookup(tree_lca->block_rmq_db,
                                         block_1,
                                         block_size_1,
                                         pos_in_block_1,
                                         pos_in_block_2 + 1);
    pos_of_min_depth = block_index_1 * tree_lca->block_rmq_db->block_size + min_pos_in_block;
  } else {
    /* Different blocks, so we need to look in each block and then between the
     * blocks to find the minimum value. */

    /* Get the positions of the minimum values in each block. For the first
     * block, start at the position and go to the end. For the second block,
     * start at the beginning and go to the position. */
    size_t min_pos_in_block_1 = BRD_lookup(tree_lca->block_rmq_db,
                                           block_1, block_size_1,
                                           pos_in_block_1,
                                           block_size_1);
    size_t min_pos_in_block_2 = BRD_lookup(tree_lca->block_rmq_db,
                                           block_2, block_size_2,
                                           0,
                                           pos_in_block_2 + 1);

    /* Get the depths at each of these positions. */ 
    size_t min_depth_in_block_1 = block_1[min_pos_in_block_1];
    size_t min_depth_in_block_2 = block_2[min_pos_in_block_2];
    
    /* Now get the block index for the block that contains the minimum value
     * between the nodes' blocks. */
    size_t min_between_tour_pos = 0;
    size_t min_depth_between_blocks = 0;
    /* We only need to check if there is at least one block between them. */
    if(block_index_2 > block_index_1 + 1) {
      size_t min_between_block_index = SparseTable_lookup(
          tree_lca->block_sparse_table, tree_lca->block_minima,
          block_index_1 + 1, block_index_2);

      /* And the position, within the tour, of the minimum value in that
      * block. */
      min_between_tour_pos = min_between_block_index *
                             tree_lca->block_rmq_db->block_size +
                             tree_lca->minima_positions[min_between_block_index];

      min_depth_between_blocks = tree_lca->euler_tour_depths[min_between_tour_pos];
    } else {
      min_depth_between_blocks = (size_t)-1;
      min_between_tour_pos = (size_t)-1;
    }

    if(min_depth_in_block_1 <= min_depth_between_blocks &&
       min_depth_in_block_1 <= min_depth_in_block_2) {
      pos_of_min_depth = block_index_1 * tree_lca->block_rmq_db->block_size + min_pos_in_block_1;
    } else if(min_depth_in_block_2 <= min_depth_between_blocks &&
              min_depth_in_block_2 <= min_depth_in_block_1) {
      pos_of_min_depth = block_index_2 * tree_lca->block_rmq_db->block_size + min_pos_in_block_2;
    } else if(min_depth_between_blocks <= min_depth_in_block_1 &&
              min_depth_between_blocks <= min_depth_in_block_2) {
      pos_of_min_depth = min_between_tour_pos;
    }
  }
  return tree_lca->euler_tour_nodes[pos_of_min_depth];
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


