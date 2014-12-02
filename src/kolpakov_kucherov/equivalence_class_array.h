#ifndef _kolpakov_kucherov_equivalence_class_tables_H_
#define _kolpakov_kucherov_equivalence_class_tables_H_

#include <stdlib.h>

/* TYPES */
#define Item_T  EquivClassItem_T
#define Array_T EquivClassArray_T

/* EquivClassItem_T represents the NextItem object described
 * by Kolpakov and Kucherov.
 *
 * Members:
 *  size_t position     :   Index in the query_string
 *  Item_T* next_item   :   Pointer to the next item in this
 *                          equivalence class
 *  Item_T* next_run    :   Pointer to the next run, as defined in
 *                          the K&K paper.
 */
typedef struct Item_T {
  size_t position;
  struct Item_T* next_item;
  struct Item_T* next_run;
}* Item_T;

/*
 * Opaque pointer to the struct that contains information for the
 * various NextItem, PreviousStartItem, etc. lookups described
 * by K&K.
 */
typedef struct Array_T* Array_T;

/* FUNCTIONS */

/* 
 * Create an EquivClassArray_T. num_equiv_classes is the total number of
 * equivalence classes that the array will hold.
 */
Array_T EquivClassArray_create(size_t num_equiv_classes);

/* Free an EquivClassArray_T. */
void    EquivClassArray_delete(Array_T* array);

/* 
 * Add an item to an EquivClassArray_T.
 *
 * Params:
 *  EquivClassArray_T        :  The array to which the item will be added.
 *  size_t equiv_class_index :  The equivalence class of the new item.
 *  size_t position          :  The position in the query string for the new
 *                              item.
 *  char* query_string       :  The query string in which the new item is
 *                              contained.
 *
 * Returns:
 *  EquivClassArray_T array  :  An array with the item added.
 */
Array_T EquivClassArray_add(Array_T array, size_t equiv_class_index, size_t position,
                            char* query_string);

/*
 * Set a new PreviousStartItem for an equivalence class.
 *
 * Params:
 *  EquivClassArray_T array     : The starting array, in which a PreviousStartItem
 *                                will be modified
 *  size_t equiv_class_index    : The equivalence class whose PreviousStartItem
 *                                will be modified
 *  EquivClassItem_T item       : The item that will be set as the new
 *                                PreviousStartItem
 * 
 * Returns:
 *  EquivClassArray_T       : The array with the new PreviousStartItem
 */
Array_T EquivClassArray_set_previous_start_item(Array_T array,
                                                size_t equiv_class_index,
                                                Item_T item);
/*
 * Get the PreviousStartItem for an equivalence class.
 *
 * Params:
 *  EquivClassArray_T array   :   The array containing the equivalence class
 *  size_t equiv_class        :   The index of the equivalence class
 *
 * Returns:
 *  EquivClassItem_T item     :   The PreviousStartItem for the equivalence
 *                                class
 */
Item_T  EquivClassArray_get_previous_start_item(Array_T array,
                                                size_t equiv_class_index);
/*
 * Get the LastItem for an equivalence class.
 */
Item_T EquivClassArray_get_last_item(Array_T array, size_t equiv_class_index);

#undef Item_T
#undef Array_T
#endif
