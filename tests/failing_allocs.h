#ifndef _failing_alloc_h
#define _failing_alloc_h

/* Required for RTLD_NEXT. */
#define _GNU_SOURCE

#include <execinfo.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BACKTRACE 50

/* 
 * Defines replacements for the allocation functions in stdlib: malloc, calloc,
 * and realloc. When the replacement functions are called with a previously
 * unobserved backtrace, then they behave as if the allocation could not be
 * accomplished, returning a null pointer. When the replacement functions are
 * called with a backtrace that has been observed before during the course of
 * the program's execution, then they behave like the equivalent stdlib
 * functions.
 *
 * While this is header-only, the API to failing_alloc is just two macros:
 *  USE_FAILING_ALLOCS  :   Override malloc, calloc, and realloc with failing
 *                          versions.
 *  FREE_FAILING_ALLOCS :   The replacement functions allocate some memory
 *                          internally to keep track of observed backtraces.
 *                          This macro frees that memory.
 */

#define USE_FAILING_ALLOCS                                              \
  void* malloc(size_t size) { return _malloc(size); }                   \
  void* calloc(size_t num, size_t size) { return _calloc(num, size); }  \
  void* realloc(void* ptr, size_t size) { return _realloc(ptr, size); }

#define FREE_FAILING_ALLOCS                                             \
  _malloc((size_t)-1);                                                  \
  _realloc(NULL, (size_t)-1);

/*
 * 
 * END OF API
 *
 */


/* Function type to compare stack arrays. */
typedef int (*CompareFailEntryFunc_T)(void** a, size_t a_size,
                                      void** b, size_t b_size);

/* Type for entry in linked list. */
struct FailStackDBEntry_T;
typedef struct FailStackDBEntry_T {
  struct FailStackDBEntry_T* next;
  void** stack;
  size_t stack_size;
} FailStackDBEntry_T;

/* Linked list that holds previously observed stacks. */
typedef struct FailStackDB_T {
  FailStackDBEntry_T* first;
  FailStackDBEntry_T* last;
  CompareFailEntryFunc_T cmp;
} FailStackDB_T;

/* Create a new FailStackDB. */
FailStackDB_T* FailStackDB_create(CompareFailEntryFunc_T cmp)
{
  void* (*stdlib_calloc)(size_t, size_t) = dlsym(RTLD_NEXT, "calloc");
  FailStackDB_T* stack_db = stdlib_calloc(1, sizeof(FailStackDB_T));
  stack_db->cmp = cmp;
  return stack_db;
}

/* Free a FailStackDB and all its entries. */
void FailStackDB_delete(FailStackDB_T* stack_db)
{
  if(stack_db) {
    FailStackDBEntry_T* entry = stack_db->first;
    FailStackDBEntry_T* next_entry = NULL;

    while(entry != NULL) {
      next_entry = entry->next;
      free(entry->stack);
      free(entry);
      entry = next_entry;
    }

    free(stack_db);
  }
}

/* Returns 1 if the backtrace is already present in the FailStackDB, else 0. */
int FailStackDB_contains(const FailStackDB_T* stack_db, void** stack, size_t stack_size)
{
  FailStackDBEntry_T* entry = stack_db->first;

  while(entry != NULL) {
    if(stack_db->cmp(entry->stack, entry->stack_size, stack, stack_size) == 0) return 1;
    entry = entry->next;
  }

  return 0;
}

/* Add a new backtrace to the DB of backtraces. */
void FailStackDB_insert(FailStackDB_T* stack_db, void** stack, size_t stack_size)
{
  void* (*stdlib_calloc)(size_t, size_t) = dlsym(RTLD_NEXT, "calloc");
  FailStackDBEntry_T* new_entry = stdlib_calloc(1, sizeof(FailStackDBEntry_T));
  new_entry->next = NULL;
  new_entry->stack = stdlib_calloc(stack_size, sizeof(void*));
  memcpy(new_entry->stack, stack, stack_size * sizeof(void*));
  new_entry->stack_size = stack_size;
  
  FailStackDBEntry_T* last_entry = stack_db->last;
  if(last_entry != NULL) {
    stack_db->last = new_entry;
    last_entry->next = new_entry;
  } else {
    stack_db->first = new_entry;
    stack_db->last = new_entry;
  }
}

/* Get the present backtrace, and return its size. */
size_t get_backtrace(void*** backtrace_ptrs)
{
  size_t bt_size;
  bt_size = backtrace(*backtrace_ptrs, MAX_BACKTRACE);
  return bt_size;
}

/* Return 0 if backtraces are equal, else return 1. */
int compare_backtraces(void** a, size_t a_size, void** b, size_t b_size)
{
  if(a_size != b_size) return 1;

  size_t i = 0;

  for(i = 0; i < a_size; i++) {
    if(a[i] != b[i]) return 1;
  }

  return 0;
}

/* The function that will replace malloc. */
void* _malloc(size_t size)
{
  /* 
   * The stack_db keeps track of all the backtraces that malloc has been called
   * in so far.
   */ 
  static FailStackDB_T* stack_db = NULL;

  /*
   * backtrace itself calls malloc, and that's bad for us because it leads to
   * an infinite loop. So, getting_backtrack keeps track of whether there's a
   * call to malloc that's trying to get the current backtrace. If so, then
   * malloc  will just behave normally rather than trying to get another
   * backtrace.
   */
  static int getting_backtrace = 0;
  
  /* Get the stdlib version of malloc. */
  void* (*stdlib_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
  
  /* 
   * If this malloc is being called by another malloc trying to get a
   * backtrace, just behave like the stdlib malloc.
   */
  if(getting_backtrace) return stdlib_malloc(size);

  /* Initialize the stack_db if it has not yet been initialized. */
  if(stack_db == NULL) {
    stack_db = FailStackDB_create(compare_backtraces);
  }
  
  /* 
   * Use (size_t)-1 as a special value to indicate that the stack_db should be
   * freed.
   */ 
  if(size == (size_t)-1) {
    if(stack_db) FailStackDB_delete(stack_db);
    return NULL;
  }

  /* Get the backtrace. */ 
  void** stack = stdlib_malloc(MAX_BACKTRACE * sizeof(void*)); 
  getting_backtrace = 1;
  size_t stack_size = get_backtrace(&stack);
  getting_backtrace = 0;
  
  /* 
   * If we've seen the backtrace before, act like stdlib malloc. Otherwise,
   * return NULL. 
   */
  void* retval = NULL;
  if(FailStackDB_contains(stack_db, stack, stack_size)) {
    retval = stdlib_malloc(size);
  } else {
    FailStackDB_insert(stack_db, stack, stack_size);
    retval = NULL;
  }
  free(stack);

  return retval;
}

/* The function that will replace calloc. */
void* _calloc(size_t num, size_t size)
{
  /* 
   * Other attempts at implementing this led to some opaque complaints about
   * consistency from ld.so. So, just malloc and memset.
   */
  void* retval = _malloc(num * size);
  if(retval) {
    memset(retval, 0, num * size);
  }
  return retval;
}

/* The function that will replace realloc. */
void* _realloc(void* ptr, size_t size)
{
  /* As in _malloc, keep track of observed backtraces. */
  static FailStackDB_T* stack_db = NULL;
  
  /* Get names for stdlib malloc and realloc. */
  void* (*stdlib_malloc)(size_t) = dlsym(RTLD_NEXT, "malloc");
  void* (*stdlib_realloc)(void*, size_t) = dlsym(RTLD_NEXT, "realloc");
  
  if(stack_db == NULL) {
    stack_db = FailStackDB_create(compare_backtraces);
  }
 
  /* Use the same sentinel value as malloc for freeing the stack_db. */ 
  if(size == (size_t)-1) {
    if(stack_db) FailStackDB_delete(stack_db);
    return NULL;
  }
  
  void** stack = stdlib_malloc(MAX_BACKTRACE * sizeof(void*)); 
  size_t stack_size = get_backtrace(&stack);
  
  void* retval = NULL;
  if(FailStackDB_contains(stack_db, stack, stack_size)) {
    retval = stdlib_realloc(ptr, size);
  } else {
    FailStackDB_insert(stack_db, stack, stack_size);
    retval = NULL;
  }
  free(stack);

  return retval;
}
#endif
