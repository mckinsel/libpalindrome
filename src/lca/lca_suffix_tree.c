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

#include "lca_suffix_tree.h"

#include "lca/euler_tour.h"
#include "lca/tour_partition.h"
#include "lca/normalized_blocks.h"
#include "lca/sparse_table.h"

#include "suffix_tree/suffix_tree_protected.h"

#include "utils/dbg.h"

#include <math.h>
#include <stdlib.h>


#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

struct LCASuffixTree_T {
  /* 
   * This has to be the first element in the struct. Changing that will break
   * everything forever.
   */ 
  struct SuffixTree_T suffix_tree;
  
  /* The Euler tour of the suffix_tree */
  EulerTour_T euler_tour;
  
  /* The partition of the node depths of the Euler tour into blocks. */
  TourPartition_T tour_partition;

  /* SparseTable for finding minima between blocks. */  
  SparseTable_T block_sparse_table;

  /* A database for range minimum queries on the blocks. */
  BlockRMQDatabase_T block_rmq_db;

}; 

LCASuffixTree_T LCASuffixTree_create(char* str, size_t length)
{
  LCASuffixTree_T lca_suffix_tree = NULL;
  LCASuffixTree_T tmp_lca_suffix_tree = NULL;

  /* First initialize as a SuffixTree_T. */
  lca_suffix_tree = (LCASuffixTree_T)SuffixTree_create(str, length);
  check_mem(lca_suffix_tree);
  
  /* Then realloc for the additional LCASuffixTree_T fields. */
  tmp_lca_suffix_tree = realloc(lca_suffix_tree, sizeof(struct LCASuffixTree_T));
  if(!tmp_lca_suffix_tree) {
    SuffixTree_T lca_as_suffix_tree = (SuffixTree_T)lca_suffix_tree;
    SuffixTree_delete(&lca_as_suffix_tree);
    lca_suffix_tree = NULL;
    check(tmp_lca_suffix_tree, "Failed reallocation for LCA fields.");
  }

  lca_suffix_tree = tmp_lca_suffix_tree;
  tmp_lca_suffix_tree = NULL;

  lca_suffix_tree->euler_tour = NULL;
  lca_suffix_tree->tour_partition = NULL;
  lca_suffix_tree->block_sparse_table = NULL;
  lca_suffix_tree->block_rmq_db = NULL;

  /* Create the Euler tour of the suffix tree. */
  lca_suffix_tree->euler_tour = EulerTour_create((SuffixTree_T)lca_suffix_tree);
  check(lca_suffix_tree->euler_tour, "Euler tour creation failed.");

  /* Create the partition of the euler tour node depths. */
  lca_suffix_tree->tour_partition = TourPartition_create(
      lca_suffix_tree->euler_tour->depths, lca_suffix_tree->euler_tour->length);
  check(lca_suffix_tree->tour_partition, "Tour partition creation failed.");

  /*
   * Create the sparse table for finding minima between blocks in the depths
   * partition.
   */
  lca_suffix_tree->block_sparse_table = SparseTable_create(
      lca_suffix_tree->tour_partition->block_minima,
      lca_suffix_tree->tour_partition->num_blocks);
  check(lca_suffix_tree->block_sparse_table, "Sparse table creation failed.");
  
  /* Finally, create the RMQ database for the blocks. */
  lca_suffix_tree->block_rmq_db = BlockRMQDatabase_create(
      lca_suffix_tree->tour_partition->block_length);
  check(lca_suffix_tree->block_rmq_db,
        "Block range minimum query database creation failed.");

  return lca_suffix_tree;

error:
  LCASuffixTree_delete(&lca_suffix_tree);
  return NULL;
}

void LCASuffixTree_delete(LCASuffixTree_T* lca_suffix_tree)
{
 if(!lca_suffix_tree) return;

 if(*lca_suffix_tree) {
   EulerTour_delete(&(*lca_suffix_tree)->euler_tour);
   TourPartition_delete(&(*lca_suffix_tree)->tour_partition);
   SparseTable_delete(&(*lca_suffix_tree)->block_sparse_table);
   BlockRMQDatabase_delete(&(*lca_suffix_tree)->block_rmq_db);
    
   SuffixTree_T lca_as_suffix_tree = (SuffixTree_T)(*lca_suffix_tree); 
   SuffixTree_delete(&lca_as_suffix_tree);
 }
}

Node_T LCASuffixTree_get_lca(LCASuffixTree_T lca_suffix_tree, Node_T node1, Node_T node2)
{
  size_t node_id1 = Node_get_index(node1);
  size_t node_id2 = Node_get_index(node2);
  /* First, we find a position of each requested node in the Euler tour arrays. */
  size_t tour_pos_1 = lca_suffix_tree->euler_tour->first_instances[node_id1];
  size_t tour_pos_2 = lca_suffix_tree->euler_tour->first_instances[node_id2];

  /* We'll need to know which one is first for some of the lookups to work
   * right. */
  size_t start_tour_pos = MIN(tour_pos_1, tour_pos_2);
  size_t end_tour_pos = MAX(tour_pos_1, tour_pos_2);

  /* Now, we want to know which block each node position falls in. */
  size_t block_index_1 = TourPartition_get_block_index(lca_suffix_tree->tour_partition,
                                                       start_tour_pos);
  size_t block_index_2 = TourPartition_get_block_index(lca_suffix_tree->tour_partition,
                                                       end_tour_pos);

  /* And the position, within its block, of each node position. */
  size_t pos_in_block_1 = TourPartition_get_pos_in_block(lca_suffix_tree->tour_partition,
                                                         start_tour_pos);
  size_t pos_in_block_2 = TourPartition_get_pos_in_block(lca_suffix_tree->tour_partition,
                                                         end_tour_pos);
  
  /* Then get the blocks themselves. Note that these are block in the node
   * depths, not the node ids. block_size_[12] give the true size of each
   * block, which may be different if it's the last block. */
  size_t* block_1 = NULL, *block_2 = NULL;

  block_1 = calloc(lca_suffix_tree->tour_partition->block_length,
                   sizeof(size_t));
  check_mem(block_1);

  size_t block_size_1 = TourPartition_get_block(
      &block_1, lca_suffix_tree->tour_partition, block_index_1,
      lca_suffix_tree->euler_tour->depths); 

  block_2 = calloc(lca_suffix_tree->tour_partition->block_length,
                   sizeof(size_t));
  check_mem(block_2);

  size_t block_size_2 = TourPartition_get_block(
      &block_2, lca_suffix_tree->tour_partition, block_index_2,
      lca_suffix_tree->euler_tour->depths);

  /* The value we want to calculate is the position of the minimum value
   * between the node positions in the depth array. */ 
  size_t pos_of_min_depth = 0;

  /* If the two nodes lie in the same block, then we just need to do an RMQ on
   * that block using the positions of each node within the block. */
  if(block_index_1 == block_index_2) {
    size_t min_pos_in_block = BlockRMQDatabase_lookup(
        lca_suffix_tree->block_rmq_db, block_1, block_size_1,
        pos_in_block_1, pos_in_block_2 + 1);
    check(min_pos_in_block != (size_t)-1, "BlockRMQDatabase lookup failed.");

    pos_of_min_depth = block_index_1 * lca_suffix_tree->tour_partition->block_length + min_pos_in_block;
  } else {
    /* Different blocks, so we need to look in each block and then between the
     * blocks to find the minimum value. */

    /* Get the positions of the minimum values in each block. For the first
     * block, start at the position and go to the end. For the second block,
     * start at the beginning and go to the position. */
    size_t min_pos_in_block_1 = BlockRMQDatabase_lookup(
        lca_suffix_tree->block_rmq_db,
        block_1, block_size_1,
        pos_in_block_1,
        block_size_1);
    check(min_pos_in_block_1 != (size_t)-1, "BlockRMQDatabase lookup failed.");

    size_t min_pos_in_block_2 = BlockRMQDatabase_lookup(
        lca_suffix_tree->block_rmq_db,
        block_2, block_size_2,
        0,
        pos_in_block_2 + 1);
    check(min_pos_in_block_2 != (size_t)-1, "BlockRMQDatabase lookup failed.");

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
          lca_suffix_tree->block_sparse_table,
          lca_suffix_tree->tour_partition->block_minima,
          block_index_1 + 1, block_index_2);

      /* And the position, within the tour, of the minimum value in that
      * block. */
      min_between_tour_pos = min_between_block_index *
                             lca_suffix_tree->tour_partition->block_length +
                             lca_suffix_tree->tour_partition->minima_positions[min_between_block_index];

      min_depth_between_blocks = lca_suffix_tree->euler_tour->depths[min_between_tour_pos];
    } else {
      min_depth_between_blocks = (size_t)-1;
      min_between_tour_pos = (size_t)-1;
    }

    if(min_depth_in_block_1 <= min_depth_between_blocks &&
       min_depth_in_block_1 <= min_depth_in_block_2) {
      pos_of_min_depth = block_index_1 * lca_suffix_tree->tour_partition->block_length + min_pos_in_block_1;
    } else if(min_depth_in_block_2 <= min_depth_between_blocks &&
              min_depth_in_block_2 <= min_depth_in_block_1) {
      pos_of_min_depth = block_index_2 * lca_suffix_tree->tour_partition->block_length + min_pos_in_block_2;
    } else if(min_depth_between_blocks <= min_depth_in_block_1 &&
              min_depth_between_blocks <= min_depth_in_block_2) {
      pos_of_min_depth = min_between_tour_pos;
    }
  }
  free(block_1);
  free(block_2);
  return lca_suffix_tree->euler_tour->nodes[pos_of_min_depth];

error:
  if(block_1) free(block_1);
  if(block_2) free(block_2);
  return NULL;
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
int LCASuffixTree_verify(LCASuffixTree_T lca_suffix_tree)
{
  /* First check that it's a correct SuffixTree_T. */
  int st_ret_val = SuffixTree_verify((SuffixTree_T)lca_suffix_tree);

  Node_T* node_array = NULL;
  Node_T* node1_to_root = NULL;
  Node_T* tmp_node1_to_root = NULL;

  if(st_ret_val != 0) {
    log_warn("lca_suffix_tree failed SuffixTree_verify.");
    return 1;
  }

  node_array = SuffixTree_create_node_array((SuffixTree_T)lca_suffix_tree);
  check_mem(node_array);
  
  /* Iterate over all possible node pairs. */
  size_t i = 0;
  size_t j = 0;
  SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes((SuffixTree_T)lca_suffix_tree);
  for(i = 0; i < num_nodes; i++) {
    for(j = 0; j < num_nodes; j++) {
      node1_to_root = NULL;
      Node_T node1 = node_array[i];
      Node_T node2 = node_array[j];
      
      /* First get the LCA node using the constant time algorithm. */
      Node_T obs_node = LCASuffixTree_get_lca(lca_suffix_tree, node1, node2);
      check(obs_node, "LCA retrieval failed.");

      Node_T exp_node = NULL;

      /* Then get the LCA node by tracing from each node back to the root. */
      node1_to_root = calloc(20, sizeof(Node_T));
      check_mem(node1_to_root);

      size_t node1_to_root_size = 20;
      size_t node1_to_root_i = 0;

      Node_T next_node = node1;
      while(next_node != 0) {
        node1_to_root[node1_to_root_i] = next_node;
        node1_to_root_i++;
        if(node1_to_root_i == node1_to_root_size) {
          tmp_node1_to_root = realloc(node1_to_root,
                                      (20 + node1_to_root_size) * sizeof(Node_T));
          check_mem(tmp_node1_to_root);

          node1_to_root = tmp_node1_to_root;
          tmp_node1_to_root = NULL;
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
        free(node_array);
        free(node1_to_root);
        return 1;
      }
      free(node1_to_root);
    }
  }
  free(node_array);
  return 0;

error:
  if(node_array) free(node_array);
  if(node1_to_root) free(node1_to_root);
  if(tmp_node1_to_root) free(tmp_node1_to_root);
  return 1;
}

