#ifndef _lca_suffix_tree_H_
#define _lca_suffix_tree_H_

#include <stdlib.h>
#include "suffix_tree/suffix_tree.h"

/* TYPES */

/*
 * An LCASuffixTree_T is a SuffixTree_T that supports constant time lowest
 * common ancestor queries. All the SuffixTree_T methods will work on an
 * LCASuffixTree_T casted to SuffixTree_T, though some functions like
 * SuffixTree_delete will do something unhelpful.
 */
typedef struct LCASuffixTree_T* LCASuffixTree_T;

/* FUNCTIONS */

LCASuffixTree_T LCASuffixTree_create(char* str, size_t length);
void            LCASuffixTree_delete(LCASuffixTree_T* tree);

Node_T          LCASuffixTree_get_lca(LCASuffixTree_T tree,
                                      Node_T node1, Node_T node2);

int             LCASuffixTree_verify(LCASuffixTree_T tree);


#endif
