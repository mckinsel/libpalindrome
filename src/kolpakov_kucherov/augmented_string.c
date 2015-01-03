#include "augmented_string.h"

#include "dbg.h"
#include "lca/lca_suffix_tree.h"

#define QPR_LENGTH(A) (2*(A)+2)

struct AugmentedString_T {
  LCASuffixTree_T tree;
  Node_T* leaf_array;
  size_t query_length;
  size_t augmented_length;
};

AugmentedString_T AugmentedString_create(char* query_string, size_t query_length)
{
  AugmentedString_T augmented_string = NULL;
  char* query_and_reverse = NULL;
  LCASuffixTree_T tree = NULL;
  Node_T* leaf_array = NULL;

  augmented_string = calloc(1, sizeof(struct AugmentedString_T));
  check_mem(augmented_string);

  query_and_reverse = malloc(QPR_LENGTH(query_length) * sizeof(char));
  check_mem(query_and_reverse);

  strncpy(query_and_reverse, query_string, query_length);
  query_and_reverse[query_length] = '#';

  size_t i = 0;
  for(i = 0; i < query_length; i++) {
    query_and_reverse[query_length + 1 + i] = query_string[query_length - 1 - i];
  }
  query_and_reverse[QPR_LENGTH(query_length) - 1] = '\0';

  tree = LCASuffixTree_create(query_and_reverse, QPR_LENGTH(query_length) - 1);
  augmented_string->tree = tree;
  check(tree, "Could not create suffix tree for the query string.");
  
  leaf_array = SuffixTree_create_leaf_array((SuffixTree_T)tree);
  augmented_string->leaf_array = leaf_array;
  check(leaf_array, "Could not create leaf array for the query string.");

  augmented_string->query_length = query_length;
  augmented_string->augmented_length = QPR_LENGTH(query_length) - 1;
  
  /* The suffix tree has a copy of the full string. */
  free(query_and_reverse);
  
  return augmented_string; 

error:
  if(augmented_string) free(augmented_string);
  if(query_and_reverse) free(query_and_reverse);
  if(leaf_array) free(leaf_array);
  LCASuffixTree_delete(&tree);

  return NULL;
}

void AugmentedString_delete(AugmentedString_T* aug_string)
{
  if(!aug_string) return;
  
  if(*aug_string) {
    LCASuffixTree_delete(&(*aug_string)->tree);
    free(*aug_string);
  }
}

size_t AugmentedString_common_prefix_suffix_length(
                       AugmentedString_T augmented_string,
                       size_t left_pos,
                       size_t right_pos)
{
  size_t adjusted_left_pos = augmented_string->augmented_length - left_pos - 1;

  Node_T node1 = augmented_string->leaf_array[adjusted_left_pos];
  Node_T node2 = augmented_string->leaf_array[right_pos];

  printf("Node1: %zu Node2: %zu\n", Node_get_index(node1), Node_get_index(node2));

  Node_T lca = LCASuffixTree_get_lca(augmented_string->tree, node1, node2);
  
  printf("LCA: %zu\n", Node_get_index(lca));

  return Node_get_edge_depth(lca);
}

LCASuffixTree_T AugmentedString_get_tree(AugmentedString_T augmented_string)
{
  return augmented_string->tree;
}

size_t AugmentedString_get_query_length(AugmentedString_T augmented_string)
{
  return augmented_string->query_length;
}

size_t AugmentedString_get_augmented_length(AugmentedString_T augmented_string)
{
  return augmented_string->augmented_length;
}
