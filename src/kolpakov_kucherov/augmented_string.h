#ifndef _augmented_string_H_
#define _augmented_string_H_

#include "lca/lca_suffix_tree.h"
#include <stdlib.h>

/* TYPES */

typedef struct AugmentedString_T* AugmentedString_T;


AugmentedString_T AugmentedString_create(char* query_string, size_t query_length);

void              AugmentedString_delete(AugmentedString_T* augmented_string);

size_t            AugmentedString_common_prefix_suffix_length(
                      AugmentedString_T augmented_string,
                      size_t left_pos,
                      size_t right_pos);

LCASuffixTree_T   AugmentedString_get_tree(AugmentedString_T augmented_string);

size_t            AugmentedString_get_query_length(AugmentedString_T augmented_string);

size_t            AugmentedString_get_augmented_length(AugmentedString_T augmented_string);

#endif
