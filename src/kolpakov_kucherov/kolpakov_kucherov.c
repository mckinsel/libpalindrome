#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolpakov_kucherov/equivalence_class.h"
#include "kolpakov_kucherov/equivalence_class_array.h"
#include "suffix_tree/suffix_tree.h"

#include "utils/dbg.h"

void length_constrained_palindromes(char* query_string, size_t query_length,
                                    size_t min_arm_length, size_t min_gap_length,
                                    size_t max_gap_length)
{
  /* Augment the string with a suffix tree of the string plus its reverse. */
  AugmentedString_T aug_string = AugmentedString_create(query_string, query_length);

  EquivClassTable_T eq_table = EquivClassTable_create(aug_string,
                                                      min_arm_length);
  
  EquivClassIndex_T num_classes = EquivClassTable_num_classes(eq_table);

  EquivClassArray_T eq_array = EquivClassArray_create(num_classes); 

  EquivClassArray_T tmp_eq_array;
  size_t j = 0;
  for(j = 0; j < query_length; j++) {

    EquivClassIndex_T left_class = EquivClassTable_reverse_lookup(eq_table, j);

    /* We can continue if we're not past min_arm_length yet. */
    if(left_class == 0) continue;
    
    tmp_eq_array = EquivClassArray_add(eq_array, left_class, j, query_string);
    check(tmp_eq_array, "Failed addition off position to equivalence class array.");
    eq_array = tmp_eq_array;

    //size_t right_class = forward_table[j];
    EquivClassIndex_T right_class = EquivClassTable_forward_lookup(eq_table, j);

    EquivClassItem_T search_item =
        EquivClassArray_get_previous_start_item(eq_array, right_class);
    
    /* If we've never seen a LeftClass for this RightClass, then we know it's
     * not a palindrome, and we can continue. */
    if(!search_item) continue;

    while(search_item->position < j - max_gap_length) {
      EquivClassItem_T next_search_item = search_item->next_item;
      if(next_search_item) {
        search_item = search_item->next_item;
      } else {
        break;
      }
    }

    eq_array = EquivClassArray_set_previous_start_item(eq_array,
                                                       right_class,
                                                       search_item);

    /* Make sure that the search_item isn't NULL before continuing with another
     * iteration. */
    while(search_item && search_item->position <= j - min_gap_length) {
      if(query_string[search_item->position] != query_string[j - 1]) {
        size_t pal_length = AugmentedString_common_prefix_suffix_length(
            aug_string, search_item->position - 1, j);
        printf("Palindrome at %zu - %zu, %zu\n", search_item->position, j, pal_length);
        printf("Palindrome bounds: (%zu-%zu), (%zu-%zu)\n",
               search_item->position - pal_length, search_item->position,
               j, j + pal_length);
      }
      search_item = search_item->next_run;
    }
  }

  EquivClassArray_delete(&eq_array);
  EquivClassTable_delete(&eq_table);
  AugmentedString_delete(&aug_string);

error:
  return;
}




