#ifndef _kolpakov_kucherov_H_
#define _kolpakov_kucherov_H_


void create_eq_class_tables(const char* query_string,
                            size_t query_length,
                            size_t substr_len, 
                            size_t** forward_table,
                            size_t** reverse_table,
                            SUFFIX_TREE** stree);

int verify_eq_class_tables(const char* query, size_t query_len, size_t substr_len,
                           const size_t* forward_table, const size_t* reverse_table);

char* append_reverse(const char* query_string, size_t query_length);

size_t* assign_equiv_classes(const char* str, size_t str_len, size_t min_len,
                             const SUFFIX_TREE* stree);


size_t* annotate_substr_classes(size_t str_len, size_t substr_len,
                                const SUFFIX_TREE* stree);

void annotate_substr_classes_dfs(const SUFFIX_TREE* stree,
                                 const NODE* node,
                                 size_t* substr_classes,
                                 size_t* class_label,
                                 size_t prev_suf_length,
                                 size_t substr_len);

int verify_substr_classes(const char* str, size_t str_len, size_t substr_len,
                          const size_t* substr_classes);
#endif
