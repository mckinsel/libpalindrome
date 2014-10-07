#include "minunit.h"
#include "test_utils.h"
#include "lca/sparse_table.h"

char* test_array()
{
  size_t arr[10] = {8, 0, 0, 3, 3, 6, 4, 9, 3, 1};
                 /* 0  1  2  3  4  5  6  7  8  9 */
  size_t arr_len = 10;
  
  SparseTable* sparse_table = SparseTable_create(arr, arr_len);
  
  size_t val = 0;
  val = SparseTable_lookup(sparse_table, arr, 5, 7);
  mu_assert(val == 6, "Didn't work.");

  val = SparseTable_lookup(sparse_table, arr, 7, 5);
  mu_assert(val == 6, "Didn't work.");

  val = SparseTable_lookup(sparse_table, arr, 5, 8);
  mu_assert(val == 6, "Didn't work.");

  val = SparseTable_lookup(sparse_table, arr, 8, 5);
  mu_assert(val == 6, "Didn't work.");

  int ret = SparseTable_verify(sparse_table, arr, arr_len); 
  mu_assert(ret == 0, "SparseTable verification failed.");

  SparseTable_delete(sparse_table);

  return NULL;
}

char* test_random_arrays()
{
  size_t* arr = calloc(300, sizeof(size_t));
  size_t arr_len = 300;

  unsigned int i = 0;
  for(i = 0; i < 20; i++) {
    random_sizes(arr, arr_len);
    SparseTable* sparse_table = SparseTable_create(arr, arr_len);
    int ret = SparseTable_verify(sparse_table, arr, arr_len); 
    mu_assert(ret == 0, "SparseTable verification failed on random array.");
    SparseTable_delete(sparse_table);
  }
  free(arr);
  return NULL;
}

char* all_tests()
{
  mu_suite_start();  

  mu_run_test(test_array);
  mu_run_test(test_random_arrays);

  return NULL;
}

RUN_TESTS(all_tests);
