#ifndef _sparse_table_H_
#define _sparse_table_H_

typedef struct SparseTable_T* SparseTable_T;


SparseTable_T SparseTable_create(const size_t* array, size_t array_size);

void          SparseTable_delete(SparseTable_T*);

size_t        SparseTable_lookup(SparseTable_T sparse_table,
                                 const size_t* array,
                                 size_t i, size_t j);

int           SparseTable_verify(SparseTable_T sparse_table,
                                 const size_t* array,
                                 size_t array_size);
#endif
