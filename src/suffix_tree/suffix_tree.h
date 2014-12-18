/******************************************************************************
Suffix Tree Version 2.1
by:         Dotan Tsadok.
Instructor: Mr. Shlomo Yona.
University of Haifa, Israel.
December 2002.

Current maintainer:
	Shlomo Yona	<shlomo@cs.haifa.ac.il>

DESCRIPTION OF THIS FILE:
This is the decleration file suffix_tree.h and it contains declarations of the
interface functions for constructing, searching and deleting a suffix tree, and
to data structures for describing the tree.

COPYRIGHT
Copyright 2002-2003 Shlomo Yona

LICENSE
This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.
*******************************************************************************/
#ifndef _suffix_tree_H_
#define _suffix_tree_H_

#include <stdlib.h>

typedef size_t SuffixTreeIndex_T;
typedef struct Node_T* Node_T;
typedef struct SuffixTree_T* SuffixTree_T;

/* SuffixTree functions */
SuffixTree_T SuffixTree_create(char* str, SuffixTreeIndex_T length);
void         SuffixTree_print(SuffixTree_T tree);
void         SuffixTree_delete(SuffixTree_T* tree);
int          SuffixTree_verify(SuffixTree_T tree);

typedef      SuffixTreeIndex_T (*NodeFunc_T)(SuffixTree_T tree,
                                             Node_T node,
                                             void* data,
                                             SuffixTreeIndex_T counter);
void         SuffixTree_walk(SuffixTree_T tree, Node_T node,
                             NodeFunc_T node_func, void* data, size_t counter);

SuffixTreeIndex_T SuffixTree_find_substring(const SuffixTree_T tree,
                                            char* query,
                                            SuffixTreeIndex_T query_length);

Node_T SuffixTree_get_root(SuffixTree_T tree);
SuffixTreeIndex_T SuffixTree_get_num_nodes(SuffixTree_T tree);
SuffixTreeIndex_T SuffixTree_get_end(SuffixTree_T tree);
size_t SuffixTree_get_string_length(SuffixTree_T tree);

Node_T* SuffixTree_create_node_array(SuffixTree_T tree);

/* Node functions */
SuffixTreeIndex_T Node_get_edge_start(Node_T node, SuffixTree_T tree);
SuffixTreeIndex_T Node_get_edge_end(Node_T node, SuffixTree_T tree);
Node_T Node_get_child(Node_T node);
Node_T Node_get_sibling(Node_T node);
Node_T Node_get_parent(Node_T node);
SuffixTreeIndex_T Node_get_index(Node_T node);

#endif
