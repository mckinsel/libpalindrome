#include "equivalence_class_array.h"
#include <stdio.h>

#include "dbg.h"

#define Item_T  EquivClassItem_T
#define List_T  EquivClassList_T
#define Array_T EquivClassArray_T

#define STARTING_NEXT_RUN_COUNT 10

/* PRIVATE TYPES */
typedef struct List_T {
  size_t length;
  Item_T first_item;
  Item_T last_item;
  Item_T previous_start_item;
  Item_T* next_run_ptrs;
  size_t num_runs;
  size_t num_allocated_runs;
}* List_T;

struct Array_T {
  size_t num_equiv_classes;
  List_T* equiv_class_lists;
};
  
/* FUNCTIONS */
void   EquivClassList_init(List_T* list);
void   EquivClassList_delete(List_T* list);
List_T EquivClassList_add(List_T list, size_t position);
List_T EquivClassList_set_new_run(List_T list, Item_T item);

/* 
 * List_T functions
 */

/* Initialize the fields of an already allocated List_T. */
void EquivClassList_init(List_T* list)
{
  *list = malloc(sizeof(struct List_T));
  (*list)->first_item = NULL;
  (*list)->last_item = NULL;
  (*list)->previous_start_item = NULL;
  (*list)->next_run_ptrs = calloc(STARTING_NEXT_RUN_COUNT, sizeof(Item_T));
  (*list)->num_allocated_runs = STARTING_NEXT_RUN_COUNT;
  (*list)->num_runs = 0;
}

/* Delete a List_T and all its items. */
void EquivClassList_delete(List_T* list)
{
  check(*list, "Attempting to delete NULL EquivClassList_T.");
  Item_T item = (*list)->first_item;
  Item_T next_item = NULL;
  while(item) {
    next_item = item->next_item;
    free(item);
    item = next_item; 
  }
  free((*list)->next_run_ptrs);
  free(*list);

error:
  return;
}  

/* Add a new item to a List_T. This does not handle setting next_run. */
List_T EquivClassList_add(List_T list, size_t position)
{
  check(list, "Attempting to add an item to a NULL list.");
  Item_T new_item = malloc(sizeof(struct Item_T));
  check_mem(new_item);
  new_item->position = position;
  new_item->next_item = NULL;
  new_item->next_run = list->next_run_ptrs[list->num_runs];
   
  if(!(list->first_item)) {
   list->first_item = new_item; 
  } else {
    list->last_item->next_item = new_item;
  }
  list->last_item = new_item;
  list->length++;

  return list;

error:
  return list;
}


List_T EquivClassList_set_new_run(List_T list, Item_T item)
{
  list->next_run_ptrs[list->num_runs] = item;
  list->num_runs++;
  if(list->num_allocated_runs > list->num_runs) {
    list->next_run_ptrs = realloc(list->next_run_ptrs, 2*list->num_allocated_runs);
  }
  list->next_run_ptrs[list->num_runs] = NULL;

  return list;
}


/*
 * Array_T functions.
 */

/* Allocate and initialize an Array_T */
Array_T EquivClassArray_create(size_t num_equiv_classes)
{
  Array_T array = calloc(1, sizeof(struct Array_T));
  check_mem(array);
  array->equiv_class_lists = malloc(num_equiv_classes * sizeof(List_T));
  array->num_equiv_classes = num_equiv_classes;
  size_t i = 0;
  for(i = 0; i < array->num_equiv_classes; i++) {
    EquivClassList_init(&array->equiv_class_lists[i]);
  }

  return array;

error:
  return NULL;
}

/* Free an Array_T, all its lists, and all its items. */
void EquivClassArray_delete(Array_T* array)
{
  check(*array, "Attempting to delete NULL EquivClassArray_T.");
  size_t i = 0;
  for(i = 0; i < (*array)->num_equiv_classes; i++) {
    EquivClassList_delete(&(*array)->equiv_class_lists[i]);
  }
  free((*array)->equiv_class_lists);
  free(*array);

error:
  return;
}

/* Add an item to the EquivClassArray. This handles both the
 * linked list aspects of adding the item as well as setting
 * next_runs
 */
Array_T EquivClassArray_add(Array_T array, size_t equiv_class_index, size_t position,
                            char* query_string)
{
  check(array, "Attempting to add item to NULL EquivClassArray_T.");
  check(query_string,
        "Attempting to add item to an EquivClassArray_T with a NULL query_string.");
  check(equiv_class_index < array->num_equiv_classes,
        "equiv_class_index is greater than number of equiv classes in the array.");
  
  List_T list = array->equiv_class_lists[equiv_class_index];
  Item_T previous_last_item = list->last_item;

  list = EquivClassList_add(list, position);

  if(previous_last_item) {
    if(query_string[position] != query_string[previous_last_item->position]) {
      EquivClassList_set_new_run(list, list->last_item);    
    }
  }
  
 return array; 

error:
  return array;
}


Array_T EquivClassArray_set_previous_start_item(Array_T array,
                                                size_t equiv_class_index,
                                                Item_T item)
{
  check(array, "Attempting to add item to NULL EquivClassArray_T.");
  check(equiv_class_index < array->num_equiv_classes,
        "equiv_class_index is greater than number of equiv classes in the array.");

  List_T list = array->equiv_class_lists[equiv_class_index];
  list->previous_start_item = item;

  return array;

error:
  return array;
}

Item_T  EquivClassArray_get_previous_start_item(Array_T array,
                                                size_t equiv_class_index)
{
  check(array, "Attempting to add item to NULL EquivClassArray_T.");
  check(equiv_class_index < array->num_equiv_classes,
        "equiv_class_index is greater than number of equiv classes in the array.");

  List_T list = array->equiv_class_lists[equiv_class_index];
  Item_T previous_start_item = list->previous_start_item;

  /* If the previous_start_item has never been set, it will be NULL. In that
   * case, return the first item. */
  if(!previous_start_item) previous_start_item = list->first_item;

  return previous_start_item;

error:
  return NULL;
}

Item_T EquivClassArray_get_last_item(Array_T array, size_t equiv_class_index)
{
  check(array, "Attempting to add item to NULL EquivClassArray_T.");
  check(equiv_class_index < array->num_equiv_classes,
        "equiv_class_index is greater than number of equiv classes in the array.");

  List_T list = array->equiv_class_lists[equiv_class_index];
  return list->last_item;

error:
  return NULL;
}

#undef Item_T
#undef List_T
#undef Array_T
