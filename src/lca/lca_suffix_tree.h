#ifndef _lca_suffix_tree_H_
#define _lca_suffix_tree_H_

#include <stdlib.h>
#include "suffix_tree/suffix_tree.h"

/* LCA means lowest common ancestor. */

/* TYPES */

/*
 * An LCASuffixTree_T is a SuffixTree_T that supports constant time lowest
 * common ancestor queries. All the SuffixTree_T methods will work on an
 * LCASuffixTree_T cast to SuffixTree_T, though some functions like
 * SuffixTree_delete will do something unhelpful.
 */
typedef struct LCASuffixTree_T* LCASuffixTree_T;

/* FUNCTIONS */

/*
 * Create an LCASuffixTree_T. Just like creating a SuffixTree_T, but with a
 * whole lot more memory allocated.
 *
 * Params:
 *  char* str           :     The string for which the suffix tree will
 *                            be created. This string cannot contain the
 *                            character '$'.
 *  size_t length       :     The length of str, not including its null-terminator.
 */
LCASuffixTree_T LCASuffixTree_create(char* str, size_t length);

/*
 * Delete the suffix tree, freeing all allocated memory.
 */
void            LCASuffixTree_delete(LCASuffixTree_T* tree);

/*
 * The function for which the LCASuffixTree_T exists. Given two nodes of an
 * LCASuffixTree_T, return the node that is their lowest common ancestor in
 * constant time.
 *
 * Params:
 *  LCASuffixTree_T tree    :   Tree containing the query nodes which will be
 *                              searched for the lowest common ancestor.
 *  Node_T node1            :   First node in the LCA query pair.
 *  Node_T node2            :   Second node in the LCA query pair.
 *
 * Returns:
 *  Node_T lca_node, the node that is the lowest common ancestor of node1 and
 *  node2.
 */
Node_T          LCASuffixTree_get_lca(LCASuffixTree_T tree,
                                      Node_T node1, Node_T node2);

/*
 * Run some slow tests on the tree to verify that 1) the tree is a correct
 * SuffixTree_T and that 2) LCA queries are working correctly.
 *
 * Returns:
 *  0 if tests pass, else 1.
 */
int             LCASuffixTree_verify(LCASuffixTree_T tree);


#endif
