#ifndef _kolpakov_kucherov_equivalence_class_private_H_
#define _kolpakov_kucherov_equivalence_class_private_H_

#include "equivalence_class.h"

void annotate_substr_classes_dfs(const SUFFIX_TREE* stree,
                                 const NODE* node,
                                 size_t* substr_classes,
                                 size_t* class_label,
                                 size_t prev_suf_length,
                                 size_t substr_len);

size_t* annotate_substr_classes(size_t str_len, size_t substr_len,
                                    const SUFFIX_TREE* stree);

int verify_substr_classes(const char* str, size_t str_len, size_t substr_len,
                          const size_t* substr_classes);

#endif
