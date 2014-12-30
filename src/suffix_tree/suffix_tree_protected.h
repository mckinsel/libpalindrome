#ifndef _suffix_tree_protected_H_
#define _suffix_tree_protected_H_

#include "suffix_tree.h"

struct SuffixTree_T
{
   SuffixTreeIndex_T         e;
   char*                     tree_string;
   SuffixTreeIndex_T         length;
   SuffixTreeIndex_T         num_nodes;
   Node_T                    root;
};

#endif  
