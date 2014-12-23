/******************************************************************************
Suffix Tree Version 2.1

AUTHORS

Dotan Tsadok
Instructor: Mr. Shlomo Yona, University of Haifa, Israel. December 2002.
Current maintainer: Shlomo Yona	<shlomo@cs.haifa.ac.il>

COPYRIGHT

Copyright 2002-2003 Shlomo Yona

LICENSE

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.
*******************************************************************************/
#include "suffix_tree.h"

#include "dbg.h"


struct Node_T
{
   /* A linked list of sons of that node */
   Node_T   left_son;
   /* A linked list of right siblings of that node */
   Node_T   right_sibling;
   /* A linked list of left siblings of that node */
   Node_T   left_sibling;
   /* A pointer to that node's father */
   Node_T   father;
   /* A pointer to the node that represents the largest
   suffix of the current node */
   Node_T   suffix_link;
   /* Index of the start position of the node's path */
   SuffixTreeIndex_T                 path_position;
   /* Start index of the incoming edge */
   SuffixTreeIndex_T                 edge_label_start;
   /* End index of the incoming edge */
   SuffixTreeIndex_T                 edge_label_end;
   /* An index for the node */
   SuffixTreeIndex_T                index;
};

struct SuffixTree_T
{
   /* The virtual end of all leaves */
   SuffixTreeIndex_T                 e;
   /* The one and only real source string of the tree. All edge-labels
      contain only indices to this string and do not contain the characters
      themselves */
   char*           tree_string;
   /* The length of the source string */
   SuffixTreeIndex_T                 length;
   /* The number of nodes in the tree */
   SuffixTreeIndex_T                 num_nodes;
   /* The node that is the head of all others. It has no siblings nor a
      father */
   Node_T                    root;
};

/* Used in function trace_string for skipping (Ukkonen's Skip Trick). */
typedef enum Skip_T     {skip, no_skip}                 Skip_T;
/* Used in function apply_rule_2 - two types of rule 2 - see function for more
   details.*/
typedef enum Rule2_T   {new_son, split}                Rule2_T;
/* Signals whether last matching position is the last one of the current edge */
typedef enum LastPos_T {last_char_in_edge, other_char} LastPos_T;

Node_T suffixless = NULL;

struct SuffixTreePath
{
   SuffixTreeIndex_T   begin;
   SuffixTreeIndex_T   end;
};

struct SuffixTreePos
{
   Node_T      node;
   SuffixTreeIndex_T   edge_pos;
};


Node_T create_node(Node_T father, SuffixTreeIndex_T start, SuffixTreeIndex_T end,
                   SuffixTreeIndex_T position)
{
   /*Allocate a node.*/
   Node_T node   = malloc(sizeof(struct Node_T));
   check_mem(node);

   node->left_son             = NULL;
   node->right_sibling        = NULL;
   node->left_sibling         = NULL;
   node->suffix_link          = NULL;
   node->father               = father;
   node->path_position        = position;
   node->edge_label_start     = start;
   node->edge_label_end       = end;
   return node;

error:
   if(node) free(node);
   return NULL;
}

Node_T find_son(const SuffixTree_T tree, const Node_T node, char character)
{
   /* Point to the first son. */
   Node_T next_node = node->left_son;
   /* scan all sons (all right siblings of the first son) for their first
   character (it has to match the character given as input to this function. */
   while(next_node != NULL && tree->tree_string[next_node->edge_label_start] != character)
   {
      next_node = next_node->right_sibling;
   }
   return next_node;
}

SuffixTreeIndex_T get_node_label_end(const SuffixTree_T tree, const Node_T node)
{
   /* If it's a leaf - return e */
   if(node->left_son == NULL)
      return tree->e;
   /* If it's not a leaf - return its real end */
   return node->edge_label_end;
}

SuffixTreeIndex_T get_node_label_length(SuffixTree_T tree, Node_T node)
{
   /* Calculate and return the lentgh of the node */
   return get_node_label_end(tree, node) - node->edge_label_start + 1;
}

char is_last_char_in_edge(SuffixTree_T tree, Node_T node,
                          SuffixTreeIndex_T edge_pos)
{
   if(edge_pos == get_node_label_length(tree,node)-1)
      return 1;
   return 0;
}

void connect_siblings(Node_T left_sib, Node_T right_sib)
{
   /* Connect the right node as the right sibling of the left node */
   if(left_sib != NULL)
      left_sib->right_sibling = right_sib;
   /* Connect the left node as the left sibling of the right node */
   if(right_sib != NULL)
      right_sib->left_sibling = left_sib;
}

Node_T apply_extension_rule_2(Node_T node, SuffixTreeIndex_T edge_label_begin,
                              SuffixTreeIndex_T edge_label_end,
                              SuffixTreeIndex_T path_pos, SuffixTreeIndex_T edge_pos,
                              Rule2_T type)
{
   Node_T new_leaf, new_internal, son;
   /*-------new_son-------*/
   if(type == new_son)                                       
   {
      /* Create a new leaf (4) with the characters of the extension */
      new_leaf = create_node(node, edge_label_begin, edge_label_end, path_pos);
      /* Connect new_leaf (4) as the new son of node (1) */
      son = node->left_son;
      while(son->right_sibling != NULL)
         son = son->right_sibling;
      connect_siblings(son, new_leaf);
      /* return (4) */
      return new_leaf;
   }
   /*-------split-------*/
   /* Create a new internal node (3) at the split point */
   new_internal = create_node(
                      node->father,
                      node->edge_label_start,
                      node->edge_label_start+edge_pos,
                      node->path_position);
   /* Update the node (1) incoming edge starting index (it now starts where node
   (3) incoming edge ends) */
   node->edge_label_start += edge_pos+1;

   /* Create a new leaf (2) with the characters of the extension */
   new_leaf = create_node(
                      new_internal,
                      edge_label_begin,
                      edge_label_end,
                      path_pos);
   
   /* Connect new_internal (3) where node (1) was */
   /* Connect (3) with (1)'s left sibling */
   connect_siblings(node->left_sibling, new_internal);   
   /* connect (3) with (1)'s right sibling */
   connect_siblings(new_internal, node->right_sibling);
   node->left_sibling = NULL;

   /* Connect (3) with (1)'s father */
   if(new_internal->father->left_son == node)
      new_internal->father->left_son = new_internal;
   
   /* Connect new_leaf (2) and node (1) as sons of new_internal (3) */
   new_internal->left_son = node;
   node->father = new_internal;
   connect_siblings(node, new_leaf);
   /* return (3) */
   return new_internal;
}

Node_T trace_single_edge(SuffixTree_T tree, Node_T node, struct SuffixTreePath str,
                         SuffixTreeIndex_T* edge_pos, SuffixTreeIndex_T* chars_found,
                         Skip_T type, int* search_done)
{
   Node_T      cont_node;
   SuffixTreeIndex_T   length, str_len;

   /* Set default return values */
   *search_done = 1;
   *edge_pos    = 0;

   /* Search for the first character of the string in the outcoming edge of
      node */
   cont_node = find_son(tree, node, tree->tree_string[str.begin]);
   if(cont_node == NULL)
   {
      /* Search is done, string not found */
      *edge_pos = get_node_label_length(tree,node)-1;
      *chars_found = 0;
      return node;
   }
   
   /* Found first character - prepare for continuing the search */
   node    = cont_node;
   length  = get_node_label_length(tree,node);
   str_len = str.end - str.begin + 1;

   /* Compare edge length and string length. */
   /* If edge is shorter then the string being searched and skipping is
      enabled - skip edge */
   if(type == skip)
   {
      if(length <= str_len)
      {
         (*chars_found)   = length;
         (*edge_pos)      = length-1;
         if(length < str_len)
            *search_done  = 0;
      }
      else
      {
         (*chars_found)   = str_len;
         (*edge_pos)      = str_len-1;
      }


      return node;
   }
   else
   {
      /* Find minimum out of edge length and string length, and scan it */
      if(str_len < length)
         length = str_len;

      for(*edge_pos=1, *chars_found=1; *edge_pos<length; (*chars_found)++,(*edge_pos)++)
      {


         /* Compare current characters of the string and the edge. If equal - 
	    continue */
         if(tree->tree_string[node->edge_label_start+*edge_pos] != tree->tree_string[str.begin+*edge_pos])
         {
            (*edge_pos)--;
            return node;
         }
      }
   }

   /* The loop has advanced *edge_pos one too much */
   (*edge_pos)--;

   if((*chars_found) < str_len)
      /* Search is not done yet */
      *search_done = 0;

   return node;
}

Node_T trace_string(SuffixTree_T tree, Node_T node, struct SuffixTreePath str,
                    SuffixTreeIndex_T* edge_pos, SuffixTreeIndex_T* chars_found,
                    Skip_T type)
{
   /* This variable will be 1 when search is done.
      It is a return value from function trace_single_edge */
   int      search_done = 0;

   /* This variable will hold the number of matching characters found in the
      current edge. It is a return value from function trace_single_edge */
   SuffixTreeIndex_T edge_chars_found;

   *chars_found = 0;

   while(search_done == 0)
   {
      *edge_pos        = 0;
      edge_chars_found = 0;
      node = trace_single_edge(tree, node, str, edge_pos, &edge_chars_found, type, &search_done);
      str.begin       += edge_chars_found;
      *chars_found    += edge_chars_found;
   }
   return node;
}

SuffixTreeIndex_T SuffixTree_find_substring(const SuffixTree_T tree,
                                            char* query,
                                            SuffixTreeIndex_T query_length)
{
   /* Starts with the root's son that has the first character of W as its
      incoming edge first character */
   Node_T node   = find_son(tree, tree->root, query[0]);
   SuffixTreeIndex_T k,j = 0, node_label_end;

   /* Scan nodes down from the root untill a leaf is reached or the substring is
      found */
   while(node != NULL)
   {
      k=node->edge_label_start;
      node_label_end = get_node_label_end(tree,node);
      
      /* Scan a single edge - compare each character with the searched string */
      while(j<query_length && k<=node_label_end && tree->tree_string[k] == query[j])
      {
         j++;
         k++;
      }
      
      /* Checking which of the stopping conditions are true */
      if(j == query_length)
      {
         /* W was found - it is a substring. Return its path starting index */
         return node->path_position;
      }
      else if(k > node_label_end)
         /* Current edge is found to match, continue to next edge */
         node = find_son(tree, node, query[j]);
      else
      {
         /* One non-matching symbols is found - W is not a substring */
         return (SuffixTreeIndex_T)-1;
      }
   }
   return (SuffixTreeIndex_T)-1;
}

void follow_suffix_link(SuffixTree_T tree, struct SuffixTreePos* pos)
{
   /* gama is the string between node and its father, in case node doesn't have
      a suffix link */
   struct SuffixTreePath      gama;
   /* dummy argument for trace_string function */
   SuffixTreeIndex_T  chars_found = 0;
   
   if(pos->node == tree->root)
   {
      return;
   }

   /* If node has no suffix link yet or in the middle of an edge - remember the
      edge between the node and its father (gama) and follow its father's suffix
      link (it must have one by Ukkonen's lemma). After following, trace down 
      gama - it must exist in the tree (and thus can use the skip trick - see 
      trace_string function description) */
   if(pos->node->suffix_link == NULL || is_last_char_in_edge(tree,pos->node,pos->edge_pos) == 0)
   {
      /* If the node's father is the root, than no use following it's link (it 
         is linked to itself). Tracing from the root (like in the naive 
         algorithm) is required and is done by the calling function SEA uppon 
         recieving a return value of tree->root from this function */
      if(pos->node->father == tree->root)
      {
         pos->node = tree->root;
         return;
      }
      
      /* Store gama - the indices of node's incoming edge */
      gama.begin      = pos->node->edge_label_start;
      gama.end      = pos->node->edge_label_start + pos->edge_pos;
      /* Follow father's suffix link */
      pos->node      = pos->node->father->suffix_link;
      /* Down-walk gama back to suffix_link's son */
      pos->node      = trace_string(tree, pos->node, gama, &(pos->edge_pos), &chars_found, skip);
   }
   else
   {
      /* If a suffix link exists - just follow it */
      pos->node      = pos->node->suffix_link;
      pos->edge_pos   = get_node_label_length(tree,pos->node)-1;
   }
}

void create_suffix_link(Node_T node, Node_T link)
{
   node->suffix_link = link;
}

void SEA(SuffixTree_T tree, struct SuffixTreePos* pos,
         struct SuffixTreePath str, SuffixTreeIndex_T* rule_applied,
         char after_rule_3)
{
   SuffixTreeIndex_T   chars_found = 0 , path_pos = str.begin;
   Node_T      tmp;
 
   /* Follow suffix link only if it's not the first extension after rule 3 was applied */
   if(after_rule_3 == 0)
      follow_suffix_link(tree, pos);

   /* If node is root - trace whole string starting from the root, else - trace last character only */
   if(pos->node == tree->root)
   {
      pos->node = trace_string(tree, tree->root, str, &(pos->edge_pos), &chars_found, no_skip);
   }
   else
   {
      str.begin = str.end;
      chars_found = 0;

      /* Consider 2 cases:
         1. last character matched is the last of its edge */
      if(is_last_char_in_edge(tree,pos->node,pos->edge_pos))
      {
         /* Trace only last symbol of str, search in the  NEXT edge (node) */
         tmp = find_son(tree, pos->node, tree->tree_string[str.end]);
         if(tmp != NULL)
         {
            pos->node      = tmp;
            pos->edge_pos   = 0;
            chars_found      = 1;
         }
      }
      /* 2. last character matched is NOT the last of its edge */
      else
      {
         /* Trace only last symbol of str, search in the CURRENT edge (node) */
         if(tree->tree_string[pos->node->edge_label_start+pos->edge_pos+1] == tree->tree_string[str.end])
         {
            pos->edge_pos++;
            chars_found   = 1;
         }
      }
   }

   /* If whole string was found - rule 3 applies */
   if(chars_found == str.end - str.begin + 1)
   {
      *rule_applied = 3;
      /* If there is an internal node that has no suffix link yet (only one may 
         exist) - create a suffix link from it to the father-node of the 
         current position in the tree (pos) */
      if(suffixless != NULL)
      {
         create_suffix_link(suffixless, pos->node->father);
         /* Marks that no internal node with no suffix link exists */
         suffixless = NULL;
      }

      #ifdef DEBUG   
         printf("rule 3 (%zu,%zu)\n",str.begin,str.end);
      #endif
      return;
   }
   
   /* If last char found is the last char of an edge - add a character at the 
      next edge */
   if(is_last_char_in_edge(tree,pos->node,pos->edge_pos) || pos->node == tree->root)
   {
      /* Decide whether to apply rule 2 (new_son) or rule 1 */
      if(pos->node->left_son != NULL)
      {
         /* Apply extension rule 2 new son - a new leaf is created and returned 
            by apply_extension_rule_2 */
         apply_extension_rule_2(pos->node, str.begin+chars_found, str.end, path_pos, 0, new_son);
         *rule_applied = 2;
         /* If there is an internal node that has no suffix link yet (only one 
            may exist) - create a suffix link from it to the father-node of the 
            current position in the tree (pos) */
         if(suffixless != NULL)
         {
            create_suffix_link(suffixless, pos->node);
            /* Marks that no internal node with no suffix link exists */
            suffixless = NULL;
         }
      }
   }
   else
   {
      /* Apply extension rule 2 split - a new node is created and returned by 
         apply_extension_rule_2 */
      tmp = apply_extension_rule_2(pos->node, str.begin+chars_found, str.end, path_pos, pos->edge_pos, split);
      if(suffixless != NULL)
         create_suffix_link(suffixless, tmp);
      /* Link root's sons with a single character to the root */
      if(get_node_label_length(tree,tmp) == 1 && tmp->father == tree->root)
      {
         tmp->suffix_link = tree->root;
         /* Marks that no internal node with no suffix link exists */
         suffixless = NULL;
      }
      else
         /* Mark tmp as waiting for a link */
         suffixless = tmp;
      
      /* Prepare pos for the next extension */
      pos->node = tmp;
      *rule_applied = 2;
   }
}

void SPA(SuffixTree_T tree, struct SuffixTreePos* pos,
         SuffixTreeIndex_T phase, SuffixTreeIndex_T* extension,
         char* repeated_extension)
{
   /* No such rule (0). Used for entering the loop */
   SuffixTreeIndex_T   rule_applied = 0;
   struct SuffixTreePath       str;
   
   /* Leafs Trick: Apply implicit extensions 1 through prev_phase */
   tree->e = phase+1;

   /* Apply explicit extensions untill last extension of this phase is reached 
      or extension rule 3 is applied once */
   while(*extension <= phase+1)            
   {
      str.begin       = *extension;
      str.end         = phase+1;
      
      /* Call Single-Extension-Algorithm */
      SEA(tree, pos, str, &rule_applied, *repeated_extension);
      /* Check if rule 3 was applied for the current extension */
      if(rule_applied == 3)
      {
         /* Signaling that the next phase's first extension will not follow a 
            suffix link because same extension is repeated */
         *repeated_extension = 1;
         break;
      }
      *repeated_extension = 0;
      (*extension)++;
   }
   return;
}

void label_nodes(Node_T node, SuffixTreeIndex_T* label)
{
  node->index = *label;
  (*label)++;
  Node_T next_node = node->left_son;
  while(next_node != NULL) {
    label_nodes(next_node, label);
    next_node = next_node->right_sibling;
  }
}

SuffixTree_T SuffixTree_create(char* str, SuffixTreeIndex_T length)
{
   SuffixTree_T  tree;
   SuffixTreeIndex_T      phase , extension;
   char          repeated_extension = 0;
   struct SuffixTreePos           pos;

   if(str == 0)
      return 0;

   /* Allocating the tree */
   tree = malloc(sizeof(struct SuffixTree_T));
   check_mem(tree);

   /* Calculating string length (with an ending $ sign) */
   tree->length         = length+1;
   
   /* Allocating the only real string of the tree */
   tree->tree_string = malloc((tree->length+1)*sizeof(char));
   check_mem(tree->tree_string);

   memcpy(tree->tree_string+sizeof(char),str,length*sizeof(char));

   /* $ is considered a uniqe symbol */
   tree->tree_string[tree->length] = '$';
   
   /* Allocating the tree root node */
   tree->root = create_node(0, 0, 0, 0);
   check_mem(tree->root);

   tree->root->suffix_link = NULL;

   /* Initializing algorithm parameters */
   extension = 2;
   phase = 2;
   
   /* Allocating first node, son of the root (phase 0), the longest path node */
   tree->root->left_son = create_node(tree->root, 1, tree->length, 1);
   suffixless       = NULL;
   pos.node         = tree->root;
   pos.edge_pos     = 0;

   /* Ukkonen's algorithm begins here */
   for(; phase < tree->length; phase++)
   {
      /* Perform Single Phase Algorithm */
      SPA(tree, &pos, phase, &extension, &repeated_extension);
   }

   SuffixTreeIndex_T counter = 0;
   label_nodes(tree->root, &counter);
   tree->num_nodes = counter;
   return tree;

error:
   if(tree->tree_string) free(tree->tree_string);
   if(tree->root) free(tree->root);
   if(tree) free(tree);
   return NULL;
}

void SuffixTree_delete_subtree(Node_T node)
{
   /* Recoursion stoping condition */
   if(node == NULL)
      return;
   /* Recoursive call for right sibling */
   if(node->right_sibling!=0)
      SuffixTree_delete_subtree(node->right_sibling);
   /* Recoursive call for first son */
   if(node->left_son!=0)
      SuffixTree_delete_subtree(node->left_son);
   /* Delete node itself, after its whole tree was deleted as well */
   free(node);
}

void SuffixTree_delete(SuffixTree_T* tree)
{
   if(*tree == NULL)
      return;
   SuffixTree_delete_subtree((*tree)->root);
   free((*tree)->tree_string);
   free(*tree);
}

void SuffixTree_print_node(SuffixTree_T tree, Node_T node1, long depth)
{
   Node_T node2 = node1->left_son;
   long  d = depth , start = node1->edge_label_start , end;
   end     = get_node_label_end(tree, node1);
   long orig_start = start;

   if(depth>0)
   {
      /* Print the branches coming from higher nodes */
      while(d>1)
      {
         printf("|");
         d--;
      }
      printf("+");
      /* Print the node itself */
      while(start<=end)
      {
         printf("%c",tree->tree_string[start]);
         start++;
      }

      printf("\t%zu\t%ld\t%ld", node1->index, orig_start, end);
      printf("\n");
   }
   /* Recoursive call for all node1's sons */
   while(node2!=0)
   {
      SuffixTree_print_node(tree,node2, depth+1);
      node2 = node2->right_sibling;
   }
}

void SuffixTree_print_full_node(SuffixTree_T tree, Node_T node)
{
   SuffixTreeIndex_T start, end;
   if(node == NULL)
      return;
   /* Calculating the begining and ending of the last edge */
   start   = node->edge_label_start;
   end     = get_node_label_end(tree, node);
   
   /* Stoping condition - the root */
   if(node->father!=tree->root)
      SuffixTree_print_full_node(tree,node->father);
   /* Print the last edge */
   while(start<=end)
   {
      printf("%c",tree->tree_string[start]);
      start++;
   }
}

void SuffixTree_print(SuffixTree_T tree)
{
   printf("\ntree_string: %s\n", tree->tree_string + 1);
   printf("\nroot\n");
   SuffixTree_print_node(tree, tree->root, 0);
   printf("\nSuffix tree of string of length %zd with %zd nodes.\n",
          tree->length, tree->num_nodes);
}

int SuffixTree_verify(SuffixTree_T tree)
{
   SuffixTreeIndex_T k,j,i;

   /* Loop for all the prefixes of the tree source string */
   for(k = 1; k<tree->length; k++)
   {
      /* Loop for each suffix of each prefix */
      for(j = 1; j<=k; j++)
      {
         /* Search the current suffix in the tree */
         i = SuffixTree_find_substring(tree, (char*)(tree->tree_string+j), k-j+1);
         if(i == (SuffixTreeIndex_T)-1)
         {
            printf("\n\nTest Results: Fail in string (%zu,%zu).\n\n",j,k);
            return 0;
         }
      }
   }
   /* If we are here no search has failed and the test passed successfuly */
   printf("Suffix tree SelfTest Result: Success.\n");
   return 1;
}

SuffixTreeIndex_T node_array_node_func(SuffixTree_T tree, Node_T node, void* vnode_array,
                                       SuffixTreeIndex_T ct)
{
  Node_T* node_array = vnode_array;
  node_array[node->index] = node;
  Node_T next_node = node->left_son;

  while(next_node != NULL) {
    node_array_node_func(tree, next_node, node_array, ct);
    next_node = next_node->right_sibling;
  }
  return 0;
}

Node_T* SuffixTree_create_node_array(SuffixTree_T tree)
{
  Node_T* node_array = calloc(tree->num_nodes, sizeof(Node_T));
  check_mem(node_array);

  SuffixTree_walk(tree, tree->root, node_array_node_func,
                  node_array, 0);
  return node_array;

error:
  if(node_array) free(node_array);
  return NULL;
}

void SuffixTree_walk(SuffixTree_T tree, Node_T node,
                     NodeFunc_T node_func, void* data,
                     SuffixTreeIndex_T counter)
{
  SuffixTreeIndex_T new_counter = node_func(tree, node, data, counter);

  Node_T next_node = node->left_son;

  while(next_node != NULL) {
    SuffixTree_walk(tree, next_node, node_func, data, new_counter);
    next_node = next_node->right_sibling;
  }
}

Node_T SuffixTree_get_root(SuffixTree_T tree)
{
  return tree->root;
}

SuffixTreeIndex_T SuffixTree_get_string_length(SuffixTree_T tree)
{
  return tree->length;
}

SuffixTreeIndex_T SuffixTree_get_num_nodes(SuffixTree_T tree)
{
  return tree->num_nodes;
}

SuffixTreeIndex_T Node_get_incoming_edge_length(Node_T node, SuffixTree_T tree)
{
  return get_node_label_end(tree, node) - node->edge_label_start;
}

int Node_is_leaf(Node_T node, SuffixTree_T tree) {
  return get_node_label_end(tree, node) == tree->e;
}

SuffixTreeIndex_T Node_get_index(Node_T node)
{
  return node->index;
}

Node_T Node_get_child(Node_T node)
{
  return node->left_son;
}

Node_T Node_get_sibling(Node_T node)
{
  return node->right_sibling;
}

Node_T Node_get_parent(Node_T node)
{
  return node->father;
}
