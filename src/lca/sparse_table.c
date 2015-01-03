#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"
#include "sparse_table.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

struct SparseTable_T {
  size_t**  table;
  size_t    nrows;
  size_t    ncols;
};

/* Create a SparseTable struct from an array.
 *
 * Inputs:
 *  size_t* array       :     Array in which RMQ queries will be made.
 *  size_t array_size   :     Size of array
 *
 * Outputs:
 *  SparseTable* sparse_table   :   SparseTabe RMQ struct
 */
SparseTable_T SparseTable_create(const size_t* array, size_t array_size)
{
  check(array_size > 0, "Cannot create a sparse table from an empty array.");
  SparseTable_T sparse_table = NULL;

  sparse_table = malloc(sizeof(struct SparseTable_T));
  check_mem(sparse_table);

  /* Get the size of the table. */
  sparse_table->nrows = array_size;
  sparse_table->ncols = (size_t)ceil(log2(array_size));
  
  /* Allocate the table */
  sparse_table->table = calloc(sparse_table->nrows, sizeof(size_t*));
  check_mem(sparse_table->table);

  size_t i = 0;
  size_t j = 0;
  for(i = 0; i < sparse_table->nrows; i++) {
    sparse_table->table[i] = calloc(sparse_table->ncols, sizeof(size_t));
    check_mem(sparse_table->table[i]);
  }

  /* Fill the table using dynamic programming. We can calculate [i, j] if we
   * have the j - 1 column, so proceed by column. */
  for(j = 0; j < sparse_table->ncols; j++) {
    for(i = 0; i < sparse_table->nrows; i++) {

      /* Simple case is the first column, that is, j == 0. That's the minimum
       * element starting in [i:i+2^0], which is just i */
      if(j == 0) {
        sparse_table->table[i][j] = i;
        continue;
      }

      /* This is the dynamic programming relation. Compare the minima of the
       * two intervals of length 2^(j-1) that span this interval. */

      /* The first interval starts at i and has length 2^(j-1) */
      size_t first_min_pos = sparse_table->table[i][j-1];
      
      /* The second interval needs to end at i + 2^j and have length 2(j-1), so
       * it starts at 1 + 2^(j-1). This might run off the end of the array. If
       * so, then we know the first_min_pos is the pos we're looking for. */
      size_t second_start = i + (1  << (j - 1));
      if(second_start >= sparse_table->nrows) {
        sparse_table->table[i][j] = first_min_pos;
        continue;
      }
      size_t second_min_pos = sparse_table->table[second_start][j - 1];
      if(array[first_min_pos] < array[second_min_pos]) {
        sparse_table->table[i][j] = first_min_pos;
      } else {
        sparse_table->table[i][j] = second_min_pos;
      }
    }
  }
  return sparse_table;

error:
  SparseTable_delete(&sparse_table);
  return NULL;
}

/* Free a SparseTable. */
void SparseTable_delete(SparseTable_T* sparse_table)
{
  if(sparse_table == NULL) return;

  if(*sparse_table) {
    if((*sparse_table)->table) {
      size_t i = 0;
      for(i = 0; i < (*sparse_table)->nrows; i++) {
        if((*sparse_table)->table[i]) free((*sparse_table)->table[i]);
      }
      free((*sparse_table)->table);
    }
    free(*sparse_table);
  }
}

/*
 * Lookup a minimum value using a SparseTable.
 * 
 * Inputs:
 *  SparseTable* sparse_table   :   The SparseTable create for the array
 *  size_t* array               :   Array in which lookup will be performed
 *  size_t i                    :   Start index
 *  size_t j                    :   End index
 *
 * Outputs:
 *  size_t min_pos    :   Position of minimum element in array[i:j]
 */ 
size_t SparseTable_lookup(SparseTable_T sparse_table,
                          const size_t* array,
                          size_t i, size_t j)
{
  check(i != j, "Cannot perform a SparseTable lookup on the empty interval "
                "[%zu:%zu].", i, j);
  size_t start = MIN(i, j);
  size_t end = MAX(i, j);

  if(end - start == 1) return start;

  size_t block_size = (size_t)floor(log2(end - start));
  size_t first_min_pos = sparse_table->table[start][block_size];
  
  /* Handle the case where the second interval is off the end of the array. */
  size_t second_start_pos = end - (1 << (block_size));
  if(second_start_pos >= sparse_table->nrows) return first_min_pos;

  size_t second_min_pos = sparse_table->table[second_start_pos][block_size];

  if(array[first_min_pos] < array[second_min_pos]) {
    return first_min_pos;
  } else {
    return second_min_pos;
  }

error:
  return (size_t)-1;
}

/*
 * Test that SparseTable_lookup is working, but finding minimum elements in all
 * subintervals using SparseTable_lookup and by just checking all the elements.
 *
 * Returns 0 if tests pass, else 1.
 */
int SparseTable_verify(SparseTable_T sparse_table, const size_t* array,
                       size_t array_size)
{
  size_t i = 0;
  size_t j = 0;

  for(i = 0; i <= array_size; i++) {
    for(j = 0; j <= array_size; j++) {
      /* i == j won't work */
      if(i == j) continue;
      
      /* Find the minimum by iterating over the whole range. */
      size_t min_val = (size_t)-1;
      size_t min_pos = 0;

      size_t start = MIN(i, j);
      size_t end = MAX(i, j);
      size_t pos = 0;
      for(pos = start; pos < end; pos++) {
        if(array[pos] <= min_val) {
          min_val = array[pos];
          min_pos = pos;
        }
      }
      
      /* And compare to the calculated value. */
      size_t test_pos = SparseTable_lookup(sparse_table, array, i, j);
      if(test_pos != min_pos) {
        log_warn("Min element position in [%zu:%zu] should be %zu, but "
                 "lookup returned %zu.", i, j, min_pos, test_pos);
        return 1;
      }
       
    }
  }
  return 0;
}
