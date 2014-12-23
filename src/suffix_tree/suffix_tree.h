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

/* TYPES */

/*
 * SuffixTree_T is the type of a suffix tree. It is opaque and is
 * manipulated by the SuffixTree functions defined below.
 */
typedef struct SuffixTree_T* SuffixTree_T;

/*
 * A suffix tree contains nodes, which are of type Node_T. These are also
 * opaque and have a number of functions defined below.
 */
typedef struct Node_T* Node_T;

/*
 * SuffixTreeIndex_T is an integer-like value that is large enough to uniquely
 * identify a node in a suffix tree.
 */
typedef size_t SuffixTreeIndex_T;

/* SuffixTree_T FUNCTIONS */

/*
 * Create a suffix tree.
 *
 * Params:
 *  char* str           :     The string for which the suffix tree will
 *                            be created. This string cannot contain the
 *                            character '$'.
 *  size_t length       :     The length of str, including its null-terminator.
 */
SuffixTree_T SuffixTree_create(char* str, SuffixTreeIndex_T length);

/*
 * Print a text representation of the tree to stdout.
 */
void         SuffixTree_print(SuffixTree_T tree);

/*
 * Delete the suffix tree, freeing all allocated memory.
 */
void         SuffixTree_delete(SuffixTree_T* tree);

/*
 * Run some slow tests on the tree to verify that it has expected properties.
 *
 * Returns:
 * 1 if tests pass, else 0.
 */
int          SuffixTree_verify(SuffixTree_T tree);

/*
 * The function pointer type used by SuffixTree_walk.
 *
 * Params:
 *  SuffixTree_T tree           :   Tree being traversed.
 *  Node_T node                 :   Current node being visited.
 *  void* data                  :   Pointer to data used/updated during the
 *                                  traversal.
 *  SuffixTreeIndex_T counter   :   Value returned by call of NodeFunc_T on
 *                                  node's parent. Or, if node is the root,
 *                                  the value given to the walk function.
 *
 * Returns:
 *  A SuffixTreeIndex_T that will be passed to calls of the NodeFunc_T on the
 *  children of node.
 */
typedef      SuffixTreeIndex_T (*NodeFunc_T)(SuffixTree_T tree,
                                             Node_T node,
                                             void* data,
                                             SuffixTreeIndex_T counter);

/*
 * Traverse the suffix tree depth-first. At each node, call a function that
 * operates on some data.
 *
 * Params:
 *  SuffixTree_T tree             : Tree to be traversed.
 *  Node_T node                   : Node at which traversal begins. Usually
 *                                  the root of the tree.
 *  NodeFunc_T node_func          : Function called at every node during the
 *                                  traversal.
 *  void* data                    : Data used/updated by node_func during
 *                                  traversal.
 *  SuffixTreeIndex_T func_retval : The return value of the node_func that
 *                                  gets passed to subsequent calls of node_func
 *                                  on the node's children. This is useful for
 *                                  doing things like keeping track of node
 *                                  depth.
 */
void         SuffixTree_walk(SuffixTree_T tree, Node_T node,
                             NodeFunc_T node_func, void* data,
                             SuffixTreeIndex_T counter);

/*
 * Find the position of a substring in the string used to create the suffix
 * tree.
 *
 * Params:
 *  SuffixTree_T tree               :   Tree to be searched.
 *  char* query                     :   Substring being searched for.
 *  SuffixTreeIndex_T query_length  :   Length of the query string.
 *
 * Returns:
 *  If the query is a substring, returns a starting position of the substring
 *  in the original string. If the query is not found, returns -1.
 */
SuffixTreeIndex_T SuffixTree_find_substring(const SuffixTree_T tree,
                                            char* query,
                                            SuffixTreeIndex_T query_length);

/*
 * Get the root of a suffix tree.
 */
Node_T SuffixTree_get_root(SuffixTree_T tree);

/*
 * Get the total number of nodes in the suffix tree.
 */
SuffixTreeIndex_T SuffixTree_get_num_nodes(SuffixTree_T tree);

/*
 * Get the length of the string from which the suffix tree was created.
 */
size_t SuffixTree_get_string_length(SuffixTree_T tree);

/*
 * Create an array of pointer to nodes of a suffix tree, such that for every
 * node, the Node_get_index(node) position in the array is a pointer to node.
 */
Node_T* SuffixTree_create_node_array(SuffixTree_T tree);


/* Node_T FUNCTIONS */

/*
 * Check whether a node is a leaf of the tree.
 *
 * Returns:
 *  1 if the node is a leaf, else 0.
 */
int Node_is_leaf(Node_T node, SuffixTree_T tree);

/*
 * Get the length of the string on the incoming edge to a node.
 */
size_t Node_get_incoming_edge_length(Node_T node, SuffixTree_T tree);

/*
 * Get a child of a node. The child returned is the "first" or leftmost child,
 * such that
 *      Node_T child = Node_get_child(node);
 *      while(child != NULL) {
 *        child = Node_get_sibling(child);
 *      }
 *
 * will iterate over all the children of the node.
 *
 * Returns NULL if the node has no children.
 */
Node_T Node_get_child(Node_T node);

/*
 * Get the sibling of a node or NULL if there are no more siblings. Siblings
 * are ordered, so just calling this repeated on a node may not iterate over
 * all siblings before it returns NULL.
 */
Node_T Node_get_sibling(Node_T node);

/*
 * Get the parent of a node or NULL if the node is the root of a tree.
 */
Node_T Node_get_parent(Node_T node);

/*
 * Get the index of a node. The index is integer-like, it is less than the
 * total number of nodes in the tree, and it uniquely identifies a node.
 */
SuffixTreeIndex_T Node_get_index(Node_T node);

#endif
