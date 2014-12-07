#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolpakov_kucherov/equivalence_class.h"
#include "kolpakov_kucherov/equivalence_class_array.h"
#include "suffix_tree/suffix_tree.h"

#include "dbg.h"

#define MIN(A, B) ((A) < (B) ? A : B)
#define MAX(A, B) ((A) > (B) ? A : B)

size_t count_equiv_classes(const size_t* equiv_class_table, size_t table_size)
{
  size_t max_value = 0;
  size_t i = 0;
  for(i = 0; i < table_size; i++) {
    if(equiv_class_table[i] > max_value) {
      max_value = equiv_class_table[i];
    }
  }
  return max_value;
}

void length_constrained_palindromes(char* query_string, size_t query_length,
                                    size_t min_arm_length, size_t min_gap_length,
                                    size_t max_gap_length)
{
  //size_t* forward_table = NULL;
  //size_t* reverse_table = NULL;
  SUFFIX_TREE* stree = NULL;
  EquivClassTable_T eq_table = EquivClassTable_create(query_string, query_length,
                                                      &stree, min_arm_length);
  
  /* First thing is to find the equivalence classes for each substring of
   * length min_arm_length. */
  //create_equiv_class_tables(query_string, query_length, min_arm_length,
  //                          &forward_table, &reverse_table, &stree);
 
  //size_t num_classes = MAX(count_equiv_classes(forward_table, query_length + 1),
  //                         count_equiv_classes(reverse_table, query_length + 1));
  EquivClassIndex_T num_classes = EquivClassTable_num_classes(eq_table);

  EquivClassArray_T eq_array = EquivClassArray_create(num_classes); 

  size_t j = 0;
  for(j = 0; j < query_length; j++) {

    //size_t left_class = reverse_table[j];
    EquivClassIndex_T left_class = EquivClassTable_reverse_lookup(eq_table, j);

    /* We can continue if we're not past min_arm_length yet. */
    if(left_class == 0) continue;
    
    eq_array = EquivClassArray_add(eq_array, left_class, j, query_string);

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
        /* TODO: Find the longest common prefix and save the palindrome */
        printf("Palindrome at %zu - %zu\n", j, search_item->position);
      }
      search_item = search_item->next_run;
    }
  }

  EquivClassArray_delete(&eq_array);
  //free(forward_table);
  //free(reverse_table);
  EquivClassTable_delete(&eq_table);
  ST_DeleteTree(stree);

}




