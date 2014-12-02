#include "equivalence_classes.h"
#include "dbg.h"
#include "kolpakov_kucherov/utils.h"


/*
 * Recursive function for depth-first traversal of suffix tree when creating
 * substring annotation.
 *
 * Inputs:
 *    SUFFIX_TREE* stree      :   Initialized suffix tree
 *    NODE* node              :   Current node in stree
 *    size_t* annotations     :   Array of substring annotions we're populating
 *    size_t* class_label     :   Pointer to current label value
 *    size_t prev_suf_length  :   Sum of lengths of suffix edges seen so far in
 *                                depth-first traversal
 *    size_t substr_len       :   Substring length used for annotation
 *
 * Outputs:
 *    None. Updates annotations and class_label
 *
 */
void annotate_substr_classes_dfs(const SUFFIX_TREE* stree,
                                 const NODE* node,
                                 size_t* substr_classes,
                                 size_t* class_label,
                                 size_t prev_suf_length,
                                 size_t substr_len)
{
  size_t edge_start = node->edge_label_start;
  size_t edge_end = get_node_label_end(stree, node);
  size_t current_suf_length = prev_suf_length + edge_end - edge_start + 1;

  if(current_suf_length >= substr_len && prev_suf_length < substr_len) {
    (*class_label)++;
  }

  if(edge_end == stree->e && current_suf_length - 1 >= substr_len) {
    size_t suffix_start = stree->e - current_suf_length;
    substr_classes[suffix_start] = *class_label;
  }

  NODE* next_node = node->sons;
  while(next_node != 0) {
    annotate_substr_classes_dfs(stree, next_node, substr_classes, class_label,
                                current_suf_length, substr_len);
    next_node = next_node->right_sibling;
  }
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
 *    size_t str_len      :   Length of string from which stree was built, not
 *                            including the null terminator
 *    size_t substr_len   :   Length of substrings used to assign class ids
 *    SUFFIX_TREE* stree  :   Suffix tree for str
 *
 * Outputs:
 *    size_t* substr_classes  :   Array of length str_len containing ids for each
 *                                position in str
 *
 * Notes:
 *    This function creates a suffix tree and then traverses it once,
 *    performing constant work at each node, so its time complexity is linear
 *    in str_len.
 */
size_t* annotate_substr_classes(size_t str_len, size_t substr_len,
                                    const SUFFIX_TREE* stree)
{
  size_t* substr_classes  = NULL;
  size_t* class_label = NULL;
  substr_classes = calloc(1, str_len * sizeof(size_t));
  check_mem(substr_classes);
  class_label = calloc(1, sizeof(size_t));
  check_mem(class_label);
  
  /* Iterate through each child of the root, passing the child to the recursive
   * helper function. */
  NODE* child = stree->root->sons;
  while(child != 0) {
    annotate_substr_classes_dfs(stree, child, substr_classes, class_label,
                                0, substr_len); /* prev_suf_length = 0 */
    child = child->right_sibling;
  }
   
  free(class_label);

  return substr_classes;

error:
  if(substr_classes) free(substr_classes);
  if(class_label) free(class_label);
  return NULL;
}
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
 *    SUFFIX_TREE** stree     :   Suffix tree of query_string
 *
 * Outputs:
 *    None, but allocates forward_table, reverse_table, and stree. So caller is
 *    responsible for freeing.
 *
 */
void create_equiv_class_tables(const char* query_string,
                               size_t query_length,
                               size_t substr_len, 
                               size_t** forward_table,
                               size_t** reverse_table,
                               SUFFIX_TREE** stree)
{
  char* query_plus_reverse = append_reverse(query_string, query_length);
  size_t qpr_length = QPR_LENGTH(query_length);
  *stree = ST_CreateTree(query_plus_reverse, qpr_length);
  size_t* substr_classes = annotate_substr_classes(qpr_length, substr_len,
                                                   *stree);
  *forward_table = calloc(query_length + 1, sizeof(size_t));
  *reverse_table = calloc(query_length + 1, sizeof(size_t));

  memcpy(*forward_table, substr_classes,
         sizeof(size_t)*(query_length - substr_len + 1));

  size_t i = 0;
  for(i = 0; i < query_length - substr_len + 1; i++) {
    (*reverse_table)[query_length - i] = substr_classes[query_length + i + 1];
  }
  
  free(query_plus_reverse);
  free(substr_classes);
}

int verify_equiv_class_tables(const char* query, size_t query_len, size_t substr_len,
                              const size_t* forward_table, const size_t* reverse_table)
{
  size_t table_len = query_len + 1;
  size_t i = 0, j = 0, k = 0;


  int same_id = 0, same_substr = 0;
  
  /* Compare forward against forward */
  for(i = 0; i < table_len; i++) {
    if(i > query_len - substr_len) {
      if(forward_table[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }
    for(j = i; j < table_len; j++) {
      same_substr = strncmp(query + i, query + j, substr_len) ? 1 : 0;
      same_id = forward_table[i] == forward_table[j] ? 0 : 1;
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

    if(i > query_len - substr_len) {
      if(forward_table[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }

    for(j = 0; j < table_len; j++) {

      if(j < substr_len) {
        if(reverse_table[j] != 0) {
          log_warn("Invalid substring given non-zero equivalence class.");
          return 1;
        }
        continue;
      }
      
      same_substr = 0;
      for(k = 0; k < substr_len; k++) {
        if(query[i + k] != query[j - 1 - k]) {
          same_substr = 1;
          break;
        }
      }
      same_id = forward_table[i] == reverse_table[j] ? 0 : 1;
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

    if(i < substr_len) {
      if(reverse_table[i] != 0) {
        log_warn("Invalid substring given non-zero equivalence class.");
        return 1;
      }
      continue;
    }

    for(j = i; j < table_len; j++) {
      same_substr = 0;
      for(k = 0; k < substr_len; k++) {
        if(query[i - 1 -k] != query[j - 1 - k]) {
          same_substr = 1;
          break;
        }
      }

      same_id = reverse_table[i] == reverse_table[j] ? 0 : 1;
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
