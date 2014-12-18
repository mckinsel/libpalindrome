#ifndef _kolpakov_kucherov_equivalence_class_private_H_
#define _kolpakov_kucherov_equivalence_class_private_H_

#include "equivalence_class.h"

#define Index_T EquivClassIndex_T
#define Table_T EquivClassTable_T

size_t* annotate_substr_classes(size_t str_length, size_t substr_length,
                                const SuffixTree_T stree);

int verify_substr_classes(const char* str, size_t str_len, size_t substr_len,
                          const size_t* substr_classes);

struct Table_T {
  Index_T* forward_classes;
  Index_T* reverse_classes;
  size_t   query_length;
};

#undef Index_T
#undef Table_T
#endif
