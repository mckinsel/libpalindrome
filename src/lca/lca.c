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

TreeLCA* TreeLCA_create(const SuffixTree_T stree)
{
  TreeLCA* tree_lca = malloc(sizeof(TreeLCA));
  check_mem(tree_lca);
  tree_lca->num_nodes = SuffixTree_get_num_nodes(stree);

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
  tree_lca->euler_tour_length = 2 * SuffixTree_get_num_nodes(stree) - 1;

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

Node_T TreeLCA_lookup(const TreeLCA* tree_lca, const Node_T node1, const Node_T node2)
{
  size_t node_id1 = Node_get_index(node1);
  size_t node_id2 = Node_get_index(node2);
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
  free(block_1);
  free(block_2);
  return tree_lca->euler_tour_nodes[pos_of_min_depth];
}

/*
 * Verify the lowest common ancestors calculated using TreeLCA by comparing
 * them against the naive n^2 algorithm.
 *
 * Inputs:
 *  SuffixTree_T stree    :   Suffix tree in which we want to find LCAs
 *  TreeLCA*  tree_lca    :   Struct used for constant time LCA lookups
 *
 * Output:
 *  0 if tests pass, else 1.
 */
int TreeLCA_verify(const SuffixTree_T stree, const TreeLCA* tree_lca)
{
  Node_T* node_array = SuffixTree_create_node_array(stree);
  
  /* Iterate over all possible node pairs. */
  size_t i = 0;
  size_t j = 0;
  for(i = 0; i < tree_lca->num_nodes; i++) {
    for(j = 0; j < tree_lca->num_nodes; j++) {
      Node_T node1 = node_array[i];
      Node_T node2 = node_array[j];
      
      /* First get the LCA node using the constant time algorithm. */
      Node_T obs_node = TreeLCA_lookup(tree_lca, node1, node2);
      Node_T exp_node = NULL;

      /* Then get the LCA node by tracing from each node back to the root. */
      Node_T* node1_to_root = calloc(20, sizeof(Node_T));
      size_t node1_to_root_size = 20;
      size_t node1_to_root_i = 0;

      Node_T next_node = node1;
      while(next_node != 0) {
        node1_to_root[node1_to_root_i] = next_node;
        node1_to_root_i++;
        if(node1_to_root_i == node1_to_root_size) {
          node1_to_root = realloc(node1_to_root,
                                  (20 + node1_to_root_size) * sizeof(size_t));
          node1_to_root_size += 20;
        }
        next_node = Node_get_parent(next_node);
      }
      
      next_node = node2;
      while(next_node != 0 && exp_node == NULL) {
        size_t k = 0;
        for(k = 0; k < node1_to_root_i; k++) {
          if(next_node == node1_to_root[k]) {
            exp_node = next_node;
            break;
          }
        }
        next_node = Node_get_parent(next_node);
      }

      if(obs_node != exp_node) {
        log_warn("Got incorrect LCA. Should get node %zu but got node %zu.",
                 Node_get_index(exp_node), Node_get_index(obs_node));
        free(node1_to_root);
        return 1;
      }
      free(node1_to_root);
    }
  }
  free(node_array);
  return 0;
}

/*
 * Create an array that maps position in a string to a leaf of a suffix tree.
 * It's important to remember that the string associated with a leaf has an
 * additional '$' char at the end that wasn't in the original string.
 *
 * Inputs:
 *    SuffixTree_T stree    :   suffix tree build from string
 *    size_t str_len        :   length of the string
 *
 * Outputs:
 *    Node_T* leaf_map   :   an array such that leaf_map[i] is a pointer to the
 *                          leaf of stree for the suffix starting at position
 *                          i in the string
 * 
 * The caller is responsible for freeing leaf_map.
 */
Node_T* map_position_to_leaf(const SuffixTree_T stree, size_t str_len)
{
  Node_T* leaf_map = calloc(str_len, sizeof(Node_T));
  check_mem(leaf_map);
  
  Node_T root = SuffixTree_get_root(stree);
  Node_T child = Node_get_child(root);
  while(child != 0) {
    map_position_to_leaf_dfs(stree, child, leaf_map, 0);
    child = Node_get_sibling(child);
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
 *  SuffixTree_T stree    :   Suffix tree of the string
 *  Node_T node            :   Current node in stree in the DFS
 *  Node_T* leaf_map       :   The array of string indices to leaf pointers
 *  size_t prev_suf_length:   The length of suffixes from node to the root
 *
 * Outputs:
 *  None, but modifies leaf_map;
 */
void map_position_to_leaf_dfs(const SuffixTree_T stree,
                              Node_T node,
                              Node_T* leaf_map,
                              size_t prev_suf_length)
{
  size_t edge_length = Node_get_incoming_edge_length(node, stree);
  size_t current_suf_length = prev_suf_length + edge_length + 1;
  
  if(Node_is_leaf(node, stree)) {
    size_t suffix_start = SuffixTree_get_string_length(stree) - current_suf_length;
    if(suffix_start + 1 != SuffixTree_get_string_length(stree)) {
      leaf_map[suffix_start] = node;
    }
    return;
  }

  Node_T next_node = Node_get_child(node);
  while(next_node != 0) {
    map_position_to_leaf_dfs(stree, next_node, leaf_map,
                             current_suf_length);
    next_node = Node_get_sibling(next_node);
  }
}

/*
 * Test the the pos_to_leaf map is correct by walking up the tree from the
 * leaves.
 *
 * Inputs:
 *    Node_T* pos_to_leaf    :   Array from string position to stree leaf
 *    SuffixTree_T stree    :   The suffix tree
 *    size_t query_len      :   Length of the string
 *
 * Outputs:
 *    0 if pos_to_leaf is correct, otherwise 1
 */
int verify_map_position_to_leaf(Node_T* pos_to_leaf,
                                const SuffixTree_T stree,
                                size_t query_len)
{
  size_t i = 0;
  size_t suffix_depth = 0;
  size_t incoming_edge_length = 0;

  for(i = 0; i < query_len; i++) {

    Node_T node = pos_to_leaf[i];
    suffix_depth = 0;
    while(Node_get_parent(node) != 0) {
      incoming_edge_length = Node_get_incoming_edge_length(node, stree);
      /* Stop when we reach the edge to the root */
      suffix_depth += incoming_edge_length + 1;
      node = Node_get_parent(node);
    }

    if(i != query_len - (suffix_depth - 1)) {
      log_warn("Leaf at position %zd has suffix of length %zd.", i, suffix_depth);
      return 1;
    }
  }
  return 0;
}


