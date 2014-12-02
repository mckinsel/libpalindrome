#ifndef _kolpakov_kucherov_equivalence_classes_H_
#define _kolpakov_kucherov_equivalence_classes_H_

#include <stdlib.h>

#include "suffix_tree/suffix_tree.h"

void create_equiv_class_tables(const char* query_string,
                               size_t query_length,
                               size_t substr_len, 
                               size_t** forward_table,
                               size_t** reverse_table,
                               SUFFIX_TREE** stree);

int verify_equiv_class_tables(const char* query, size_t query_len, size_t substr_len,
                              const size_t* forward_table, const size_t* reverse_table);

#endif
