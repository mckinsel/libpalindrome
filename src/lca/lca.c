#include <stdlib.h>

#include "dbg.h"
#include "suffix_tree/suffix_tree.h"

#include "lca.h"

/*
 * Count the number of nodes in the subtree of node.
 */
void count_nodes(const NODE* node, DBL_WORD* counter)
{
  NODE* next_node = node->sons;
  while(next_node != 0) {
    (*counter)++;
    count_nodes(next_node, counter);
    next_node = next_node->right_sibling;
  }
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

