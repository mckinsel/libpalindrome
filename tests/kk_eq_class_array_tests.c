#include "minunit.h"
#include "test_utils.h"
#include "kolpakov_kucherov/equivalence_class_array.h"


char* test_array_add()
{
  char str[] = "BANANA";
  EquivClassArray_T array;
  array = EquivClassArray_create(5);

  EquivClassArray_add(array, 0, 0, str);
  EquivClassItem_T item = EquivClassArray_get_last_item(array, 0);
  
  mu_assert(item->position == 0, "Incorrect position for LastItem.");
  mu_assert(item->next_run == NULL, "Incorrect NextRun for LastItem.");
  
  EquivClassArray_add(array, 0, 4, str);
  item = EquivClassArray_get_last_item(array, 0);
  mu_assert(item->position == 4, "Incorrect position for LastItem.");

  EquivClassArray_delete(&array);
  return NULL;
}


char* test_array_run()
{
  char str[] = "BANANA";
  EquivClassArray_T array;
  array = EquivClassArray_create(5);

  EquivClassArray_add(array, 2, 1, str);
  EquivClassArray_add(array, 2, 3, str);
  EquivClassArray_add(array, 2, 4, str);

  EquivClassItem_T item = EquivClassArray_get_last_item(array, 2);
  mu_assert(item->position == 4, "Incorrect position for LastItem.");
  mu_assert(item->next_run == NULL, "Incorrect NextRun for LastItem.");
  
  EquivClassArray_delete(&array);
  return NULL;
}  


char* test_previous_start_items()
{
  char str[] = "BANANA";
  EquivClassArray_T array;
  array = EquivClassArray_create(5);

  EquivClassArray_add(array, 2, 1, str);
  EquivClassArray_add(array, 2, 3, str);
  EquivClassArray_add(array, 2, 4, str);
  
  EquivClassItem_T item = EquivClassArray_get_previous_start_item(array, 2);
  mu_assert(item->position == 1, "Incorrect position for PreviousStartItem.");

  EquivClassArray_set_previous_start_item(array, 2, item->next_item); 
  item = EquivClassArray_get_previous_start_item(array, 2);
  mu_assert(item->position == 3, "Incorrect position for PreviousStartItem.");

  EquivClassArray_delete(&array);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_array_add);
  mu_run_test(test_array_run);
  mu_run_test(test_previous_start_items);
  return NULL;
}

RUN_TESTS(all_tests);
