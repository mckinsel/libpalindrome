#ifndef _lca_H_
#define _lca_H_

#include "lca/euler_tour.h"
#include "lca/normalized_blocks.h"
#include "lca/sparse_table.h"
#include "suffix_tree/suffix_tree.h"

typedef struct {
  
  /* Number of vertexes in the tree */
  size_t num_nodes;
  
  /* The node ids as they're visited in the Euler tour. */
  NODE** euler_tour_nodes;

  /* The depths of nodes as they are visited in an Euler tour of the tree. */
  size_t* euler_tour_depths;

  /* The size of the euler tour depths array. */
  size_t euler_tour_length;

  /* Position of the first occurrence of each node id in euler_tour_depths. */
  size_t* node_id_pos_in_tour;
  
  /* Euler tour depths is partitioned into blocks. block_minima contains the
   * minimum value each block.  */
  size_t* block_minima;
  /* minima_positions stores the position of the minimum in each block. */
  size_t* minima_positions;
  /* Number of blocks length of block_minima */
  size_t num_blocks;

  /* SparseTable for finding minima between blocks. */  
  SparseTable* block_sparse_table;

  /* A database for range minimum queries on the blocks. */
  BlockRMQDatabase* block_rmq_db;

} TreeLCA; 

TreeLCA* TreeLCA_create(const SUFFIX_TREE* stree);
void TreeLCA_delete(TreeLCA* tree_lca);
NODE* TreeLCA_lookup(const TreeLCA* tree_lca, const NODE* node1, const NODE* node2);
int TreeLCA_verify(const SUFFIX_TREE* stree, const TreeLCA* tree_lca);


NODE** map_position_to_leaf(const SUFFIX_TREE* stree, size_t str_len);

void map_position_to_leaf_dfs(const SUFFIX_TREE* stree,
                              NODE* node,
                              NODE** leaf_map,
                              size_t prev_suf_length);

int verify_map_position_to_leaf(NODE** pos_to_leaf,
                                const SUFFIX_TREE* stree,
                                size_t query_len);
#endif
