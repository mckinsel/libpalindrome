#ifndef _kolpakov_kucherov_equivalence_class_H_
#define _kolpakov_kucherov_equivalence_class_H_

#include <stdlib.h>

#include "suffix_tree/suffix_tree.h"
#include "augmented_string.h"

#define Index_T EquivClassIndex_T
#define Table_T EquivClassTable_T

typedef size_t Index_T;
typedef struct Table_T* Table_T; 

Table_T EquivClassTable_create(AugmentedString_T augmented_string,
                               Index_T substr_length);

void    EquivClassTable_delete(Table_T* table);

Index_T EquivClassTable_forward_lookup(Table_T table, size_t query_string_pos);

Index_T EquivClassTable_reverse_lookup(Table_T table, size_t query_string_pos);

Index_T EquivClassTable_num_classes(Table_T table);

int     EquivClassTable_verify(char* query_string,
                               Index_T query_length,
                               Table_T table,
                               Index_T substr_length);

#undef Index_T
#undef Table_T
#endif
