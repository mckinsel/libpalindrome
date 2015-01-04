#include "equivalence_class.h"
#include "utils/dbg.h"
#include "augmented_string.h"

#define Index_T EquivClassIndex_T
#define Table_T EquivClassTable_T

struct SubstrClassDFS {
  size_t  substr_length;
  size_t* substr_classes;
  size_t* class_label;
};


/*
 * Helper function for depth-first traversal of suffix tree when creating
 * substring annotation.
 *
 * Inputs:
 *    SuffixTree_T stree      :   Initialized suffix tree
 *    NODE* node              :   Current node in stree
 *    void* data              :   Void* to be a proper NODE_FUNC_T, but a
 *                                pointer to a struct SubstrClassDFS that will
 *                                get updated during traversal
 *    size_t prev_suf_length  :   Sum of lengths of suffix edges seen so far in
 *                                depth-first traversal
 *
 * Outputs:
 *    size_t current_suf_length : prev_suf_length plus the length of the suffix
 *                                portion associated with this node
 */
size_t annotate_substr_node_func(const SuffixTree_T stree, const Node_T node,
                                 void *data, size_t prev_suf_length)
{

  if(node == SuffixTree_get_root(stree)) return 0;
  struct SubstrClassDFS* dfs_data = data;
  size_t edge_length = Node_get_incoming_edge_length(node, stree);
  size_t current_suf_length = prev_suf_length + edge_length;

  if(current_suf_length >= dfs_data->substr_length &&
      prev_suf_length < dfs_data->substr_length) {
    (*dfs_data->class_label)++;
  }

  if(Node_is_leaf(node, stree) && current_suf_length - 1 >= dfs_data->substr_length) {
    size_t suffix_start = SuffixTree_get_string_length(stree) - current_suf_length;
    dfs_data->substr_classes[suffix_start] = *dfs_data->class_label;
  }

  return current_suf_length;
}

/* 
 * Given a string and a length, assign an id to each position in the string
 * such that two positions have the same id if and only if substrings of the
 * given length starting at both positions are equal. For example, consider the
 * string "MISSISSIPPI" and length 3:
 *
 *     M I S S I S S I P P I
 *     1 2 3 4 2 3 5 6 7 0 0
 *
 * The substrings of length 3 starting at position 1 and 4 are both "ISS", so
 * that have to have the same id. The actual value of the id doesn't matter,
 * just that they equal. Positions 9 and 10 have no substring of length three,
 * so they are assigned id 0.
 *
 * Inputs:
 *    size_t str_length     :   Length of string from which stree was built, not
 *                              including the null terminator
 *    size_t substr_length  :   Length of substrings used to assign class ids
 *    SuffixTree_T stree    :   Suffix tree for str
 *
 * Outputs:
 *    size_t* substr_classes  :   Array of length str_len containing ids for each
 *                                position in str
 *
 * Notes:
 *    This function traverses a suffix tree once,
 *    performing constant work at each node, so its time complexity is linear
 *    in str_len.
 */
size_t* annotate_substr_classes(size_t str_length, size_t substr_length,
                                const SuffixTree_T stree)
{
  struct SubstrClassDFS* dfs_data = calloc(1, sizeof(struct SubstrClassDFS));
  check_mem(dfs_data);

  dfs_data->substr_length = substr_length;
  dfs_data->class_label = calloc(1, sizeof(size_t));
  check_mem(dfs_data->class_label);

  dfs_data->substr_classes = calloc(1, str_length * sizeof(size_t));
  check_mem(dfs_data->substr_classes);
   
  SuffixTree_walk(stree, SuffixTree_get_root(stree), annotate_substr_node_func,
                  dfs_data, 0);

  size_t* substr_classes = dfs_data->substr_classes;
  free(dfs_data->class_label);
  free(dfs_data);

  return substr_classes;

error:
  if(dfs_data) {
    if(dfs_data->substr_classes) free(dfs_data->substr_classes);
    if(dfs_data->class_label) free(dfs_data->class_label);
    free(dfs_data);
  }
  return NULL;
}

/*
 * Test the substring classes found by annotate_substr_classes. Check that
 * every pair of identical substrings has the same class id and every pair of
 * different substrings has different class ids.
 *
 * Inputs:
 *    char* str              :   String containing substrings
 *    size_t str_len         :   Length of str, not including null terminator
 *    size_t substr_len      :   Length of substrings to be compared
 *    size_t* substr_classes :   Array of substring class ids
 *
 * Outputs:
 *    int ret_val   :   0 if all tests pass, otherwise 1
 *
 */
int verify_substr_classes(const char* str, size_t str_len, size_t substr_len,
                          const size_t* substr_classes)
{
  size_t i = 0, j = 0;
  int same_substr = 0, same_id = 0;
  for(i = 0; i < str_len; i++) {

    /* Substring that runs off the end of the string must be class zero. */
    if(i + substr_len > str_len) {
      if(substr_classes[i] != 0) {
        log_warn("Position %zd should have substr class 0, but it has "
                 "class %zd.", i, substr_classes[i]);
        return 1;
      }
      continue;
    /* And if it does not run off the end, class must not be zero. */
    } else {
      if(substr_classes[i] == 0) {
        log_warn("Position %zd should not be substr class 0.", i);
        return 1;
      }
    }
    
    for(j = 0; j < str_len - substr_len; j++) {
      same_substr = strncmp(str + i, str + j, substr_len) ? 1 : 0;
      same_id = substr_classes[i] == substr_classes[j] ? 0 : 1;
      if(same_substr != same_id) {
        log_warn("Comparing positions %zd and %zd: same_substr is %d, but same_id is %d",
                 i, j, same_substr, same_id);
        return 1;
      }
    }
  }

  return 0;
}

struct Table_T {
  Index_T* forward_classes;
  Index_T* reverse_classes;
  size_t   query_length;
};

/*
 * Create the two equivalence class tables for a string and a given minimum
 * length, as described by Kolpakov and Kucherov.
 *
 * There are two, one for the forward direction, andi the other for the
 * reverse. For example, consider BANANA
 *
 *      B A N A N A   string
 *      0 1 2 3 4 5   index
 *
 * forward_table[i] returns the equivalence class for string[i:i+min_len],
 * and reverse_table[i] returns the equivalence class for
 * string[i-min_len:i][::-1], using Python slice syntax. So the correct values
 * for BANANA with min_len of 3 are
 *
 *  forward:              reverse:
 *    0   :   1 (BAN)            0   :   0
 *    1   :   2 (ANA)            1   :   0
 *    2   :   3 (NAN)            2   :   0
 *    3   :   2 (ANA)            3   :   7 (NAB)
 *    4   :   0                  4   :   2 (ANA)
 *    5   :   0                  5   :   3 (NAN)
 *    6   :   0                  6   :   2 (ANA)
 *
 * forward[1] refers to ANA in B[ANA]NA. reverse[4] refers to ANA in AN[ANA]B.
 * So, the have the same value. Note that zero is a special value indicating
 * that there is not valid substring in that direction at that position.
 *
 * Inputs:
 *    char* query_string      :   String in which equivalence classes will be
 *                                found
 *    size_t query_length     :   Length of query_string, not including null
 *                                terminator
 *    size_t substr_length    :   Length of substrings in the equivalence
 *                                classes
 *    size_t** forward_table  :   The RightClass table from the paper
 *    size_t** reverse_table  :   The LeftClass table from the paper
 *    SuffixTree_T* stree     :   Suffix tree of query_string
 *
 * Outputs:
 *    None, but allocates forward_table, reverse_table, and stree. So caller is
 *    responsible for freeing.
 *
 */

Table_T EquivClassTable_create(AugmentedString_T augmented_string,
                               Index_T substr_length)
{
  Table_T table = NULL;
  size_t* substr_classes = NULL;

  table = calloc(1, sizeof(struct Table_T));
  check_mem(table);
  size_t query_length = AugmentedString_get_query_length(augmented_string);
  table->query_length = query_length;

  substr_classes = annotate_substr_classes(
      AugmentedString_get_augmented_length(augmented_string),
      substr_length,
      (SuffixTree_T)AugmentedString_get_tree(augmented_string));
  check(substr_classes, "Failed annotation of substring equivalence classes.");


  table->forward_classes = calloc(query_length + 1, sizeof(Index_T));
  check_mem(table->forward_classes);

  table->reverse_classes = calloc(query_length + 1, sizeof(Index_T));
  check_mem(table->reverse_classes);

  memcpy(table->forward_classes, substr_classes,
         sizeof(Index_T)*(query_length - substr_length + 1));

  size_t i = 0;
  for(i = 0; i < query_length - substr_length + 1; i++) {
    (table->reverse_classes)[query_length - i] = substr_classes[query_length + i + 1];
  }
  
  free(substr_classes);
  
  return table;

error:
  if(substr_classes) free(substr_classes);
  if(table) {
    if(table->forward_classes) free(table->forward_classes);
    if(table->reverse_classes) free(table->reverse_classes);
    free(table);
  }
  return NULL;
}

void EquivClassTable_delete(Table_T* table)
{
  check(*table, "Attempting to delete NULL EquivClassTable_T.");

  if((*table)->forward_classes) free((*table)->forward_classes);
  if((*table)->reverse_classes) free((*table)->reverse_classes);
  free(*table);

error:
  return;

}  

Index_T EquivClassTable_forward_lookup(Table_T table, size_t query_string_pos)
{
  check(query_string_pos <= table->query_length,
        "Forward lookup position exceeds length of query string");

  return table->forward_classes[query_string_pos];

error:
  return (Index_T)-1;
}

Index_T EquivClassTable_reverse_lookup(Table_T table, size_t query_string_pos)
{
  check(query_string_pos <= table->query_length,
        "Reverse lookup position exceeds length of query string");

  return table->reverse_classes[query_string_pos];

error:
  return (Index_T)-1;
}

int     EquivClassTable_verify(char*   query_string,
                               Index_T query_length,
                               Table_T table,
                               Index_T substr_length)
{
  size_t table_len = query_length + 1;
  size_t i = 0, j = 0, k = 0;


  int same_id = 0, same_substr = 0;
  
  /* Compare forward against forward */
  for(i = 0; i < table_len; i++) {
    if(i > query_length - substr_length) {
      if(table->forward_classes[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }
    for(j = i; j < table_len; j++) {
      same_substr = strncmp(query_string + i, query_string + j, substr_length) ? 1 : 0;
      same_id = table->forward_classes[i] == table->forward_classes[j] ? 0 : 1;
      if(same_substr != same_id) {
        log_warn("Comparing positions %zd and %zd in the forward_table: "
                 "same_substr is %d, but same_id is %d",
                 i, j, same_substr, same_id);
        return 1;
      }
    }
  }

  /* Compare forward against reverse. i indexes forward, j indexes reverse */
  for(i = 0; i < table_len; i++) {

    if(i > query_length - substr_length) {
      if(table->forward_classes[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }

    for(j = 0; j < table_len; j++) {

      if(j < substr_length) {
        if(table->reverse_classes[j] != 0) {
          log_warn("Invalid substring given non-zero equivalence class.");
          return 1;
        }
        continue;
      }
      
      same_substr = 0;
      for(k = 0; k < substr_length; k++) {
        if(query_string[i + k] != query_string[j - 1 - k]) {
          same_substr = 1;
          break;
        }
      }
      same_id = table->forward_classes[i] == table->reverse_classes[j] ? 0 : 1;
      if(same_substr != same_id) {
        log_warn("Comparing positions %zd and %zd in the forward and reverse table: "
                 "same_substr is %d, but same_id is %d",
                 i, j, same_substr, same_id);
        return 1;
      }
    }
  }

  /* Compare reverse against reverse */
  for(i = 0; i < table_len; i++) {

    if(i < substr_length) {
      if(table->reverse_classes[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }

    for(j = i; j < table_len; j++) {
      same_substr = 0;
      for(k = 0; k < substr_length; k++) {
        if(query_string[i - 1 -k] != query_string[j - 1 - k]) {
          same_substr = 1;
          break;
        }
      }

      same_id = table->reverse_classes[i] == table->reverse_classes[j] ? 0 : 1;
      if(same_substr != same_id) {
        log_warn("Comparing positions %zd and %zd in the reverse table: "
                 "same_substr is %d, but same_id is %d",
                 i, j, same_substr, same_id);
        return 1;
      }
    }
  }

  return 0;
}

Index_T EquivClassTable_num_classes(Table_T table)
{

  Index_T max_value = 0;
  size_t i = 0;
  for(i = 0; i < table->query_length + 1; i++) {
    if(table->forward_classes[i] > max_value) {
      max_value = table->forward_classes[i];
    }
    if(table->reverse_classes[i] > max_value) {
      max_value = table->reverse_classes[i];
    }
  }
  return max_value + 1;
}


#undef Index_T
#undef Table_T
