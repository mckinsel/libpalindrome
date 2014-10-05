#ifndef _sparse_table_H_
#define _sparse_table_H_
typedef struct
{
  size_t**  table;
  size_t    nrows;
  size_t    ncols;
} SparseTable;


SparseTable* SparseTable_create(const size_t* array, size_t array_size);

void SparseTable_delete(SparseTable*);

size_t SparseTable_lookup(const SparseTable* sparse_table, const size_t* array,
                          size_t i, size_t j);

int  SparseTable_verify(const SparseTable* sparse_table, const size_t* array,
                        size_t array_size);
#endif
