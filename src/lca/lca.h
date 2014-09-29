#ifndef _lca_H_
#define _lca_H_

void count_nodes(const NODE* node, DBL_WORD* counter);

NODE** map_position_to_leaf(const SUFFIX_TREE* stree, size_t str_len);

void map_position_to_leaf_dfs(const SUFFIX_TREE* stree,
                              NODE* node,
                              NODE** leaf_map,
                              size_t prev_suf_length);

int verify_map_position_to_leaf(NODE** pos_to_leaf,
                                const SUFFIX_TREE* stree,
                                size_t query_len);
#endif
