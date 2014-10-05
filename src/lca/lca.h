#ifndef _lca_H_
#define _lca_H_

#include "suffix_tree/suffix_tree.h"

/* Functions to create arrays for RMQ from the suffix tree. */
void prepare_rmq_arrays(const SUFFIX_TREE* stree, DBL_WORD** depths,
                        DBL_WORD** first_instances);
void euler_tour(NODE* node, DBL_WORD depth, DBL_WORD* pos_in_tour,
                DBL_WORD* depths, DBL_WORD* first_instances);
int verify_rmq_arrays(const SUFFIX_TREE* stree, const DBL_WORD* depths,
                      const DBL_WORD* first_instances);

/* Functions for partitioning the depth array. */
size_t get_partition(size_t pos, size_t n);
size_t get_partition_size(size_t n);
size_t get_num_partitions(size_t n);

/* Function to get block minima, the A` and B arrays from the paper. */
void get_partition_minima(const size_t* depths, size_t depths_size,
                          size_t** block_minima, size_t** minima_positions);


NODE** map_position_to_leaf(const SUFFIX_TREE* stree, size_t str_len);

void map_position_to_leaf_dfs(const SUFFIX_TREE* stree,
                              NODE* node,
                              NODE** leaf_map,
                              size_t prev_suf_length);

int verify_map_position_to_leaf(NODE** pos_to_leaf,
                                const SUFFIX_TREE* stree,
                                size_t query_len);
#endif
