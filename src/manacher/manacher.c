#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"

#define MIN(A, B) ((A) < (B) ? A : B)
#define MAX(A, B) ((A) > (B) ? A : B)

/* Macros for converting a palidrome center (C) and radius (R) to
 * positions it the query string.
 */
#define QUERY_START(C, R) ((C) / 2 - (R))
#define QUERY_END(C, R) ((C) % 2 == 0 ? ((C) / 2 + (R) - 1) : ((C) / 2 + (R)))

/*
 * Run Manacher's algorithm on a string, returning the longest palindrome
 * centered at each position in the string. Since palindromes can be centered
 * between two characters, there are 2 * query_length + 1 possible centers:
 *
 *        B A N A N A
 *       0123456789012
 * 
 * and a palindrome radius can be calculated at each:
 *
 *        B A N A N A
 *       0000010201000 
 *
 * Input:
 *    char* query_string    :   String to be search for palindromes
 *    size_t query_length   :   Length of query_string, not including null
 *                              terminator
 * 
 * Output:
 *    size_t* pal_radii     :   Radius of maximal palindrome at each possible
 *                              center
 */
size_t* manacher(char* query_string, size_t query_length)
{
  
  size_t pal_radii_length = 2 * query_length + 1;
  size_t* pal_radii = calloc(pal_radii_length, sizeof(size_t));
  check_mem(pal_radii);
  
  /* This is an index to pal_radii, it tracks which element we're calculating
   */
  size_t pal_radii_pos = 0;
  
  /* This is the index in pal_radii of the palindrome that may span over
   * pal_radii_pos */
  size_t current_pal_center = 0;
  
  
  /* Don't check the first and last elements of P, those are always zero, and
   * they screw up our indexing */
  for(pal_radii_pos = 1; pal_radii_pos < pal_radii_length - 1; pal_radii_pos++) {
    
    /* Get the radius of the current spanning palindrome, but in terms of the
     * pal_radii array. So, multiply by two */
    size_t current_pal_radius = 2 * pal_radii[current_pal_center];
    
    /* How far the current palindrome extends in the pal_radii array */
    size_t right_boundary = current_pal_center + current_pal_radius;
    
    /* If our center falls within the extent, we know a floor for its radius */
    if(right_boundary > pal_radii_pos) {
      size_t mirror_pos = 2 * current_pal_center - pal_radii_pos;
      size_t max_radius = (right_boundary - pal_radii_pos) / 2;
      pal_radii[pal_radii_pos] = MIN(max_radius, pal_radii[mirror_pos]);
    }
    
    while(1) {
      /* Check if the palindrome has reached the end of the string. */
      if((QUERY_START(pal_radii_pos, pal_radii[pal_radii_pos])) == 0) break;
      if((QUERY_END(pal_radii_pos, pal_radii[pal_radii_pos])) == query_length - 1) break;
      
      if(query_string[QUERY_START(pal_radii_pos, pal_radii[pal_radii_pos] + 1)] ==
         query_string[QUERY_END(pal_radii_pos, pal_radii[pal_radii_pos] + 1)]) {
        pal_radii[pal_radii_pos]++;
      } else {
        break;
      }
    }
       
    if(2 * pal_radii[pal_radii_pos] + pal_radii_pos > right_boundary) {
      current_pal_center = pal_radii_pos;
    }
  }

  return pal_radii;

error:
  if(pal_radii) free(pal_radii);
  return NULL;
}

/*
 * Check that the calculated palindrome radii are correct. Each palindrome is
 * checked to see that it is the same as its reverse and that it cannot be
 * extended.
 *
 * Input:
 *    char* query_string    :   String containing palindromes
 *    size_t query_length   :   Length of query_string, not including null
 *                              terminator
 *    size_t* pal_radii     :   Maximal palindrome radii to be tested for
 *                              correctness
 * 
 * Output:
 *    int ret_code          :   0 if all palindromes are correct, otherwise 1
 */
int verify_palindrome_radii(char* query_string, size_t query_length, size_t* pal_radii)
{
  size_t i = 0;
  
  /* First check that the first and last radii are zero */
  if (pal_radii[0] != 0) {
    log_warn("First element should be zero, but it is %zd", pal_radii[0]);
    return 1;
  }
  if (pal_radii[2 * query_length] != 0) {
    log_warn("Last element should be zero, but it is %zd", pal_radii[2 * query_length]);
    return 1;
  }
  
  /* Now step through each index, verifying the each palindrome is the same as
   * its reverse and that the palindrome cannot be extended */
  for(i = 1; i < 2 * query_length; i++) {
    
    size_t pal_radius = pal_radii[i];
    
    size_t query_start = QUERY_START(i, pal_radii[i]);
    size_t query_end = QUERY_END(i, pal_radii[i]);
    
    /* Try to extend the palindrome, but only if it won't run off the end of
     * the query */
    if(query_start > 0 && query_end < query_length - 1) {
      if(query_string[query_start - 1] == query_string[query_end + 1]) {
        log_warn("Palindrome centered at %zd with radius %zd is not maximal.",
                 i, pal_radius);
        return 1;
      }
    }
    
    if(pal_radius == 0) continue;

    size_t j = 0;
    for(j = 0; j < query_end - query_start; j++) {
       
      if(query_string[query_start + j] != query_string[query_end - j]) {
        log_warn("Failed comparison for radius at %zd, starting at %zd and ending at %zd"
                 ", comparing %c and %c, %zd %zd",
                 i, query_start, query_end, query_string[query_start + j], query_string[query_end - j],
                 query_start + j, query_end - j);
        return 1;
      }
    }

  }

  return 0;
}

/*
 * Get the longest palindrome in a string based on the calculated palindrome
 * radii.
 *
 * Inputs:
 *    char* query_string  :   String containing palindromes
 *    size_t query_length :   Length of query_string, not including null terminator
 *    size_t* pal_radii   :   Radius of maximal palindrome at each center in
 *                            query_string
 *
 * Outputs:
 *    char* longest       :   Null-terminated string of longest palindrome in
 *                            query_string
 */
char* longest_palindrome(char* query_string, size_t query_length, size_t* pal_radii)
{
  size_t max_val = 0;
  size_t max_index = 0;
  size_t i = 0;
  for(i = 0; i < 2 * query_length + 1; i++) {
    if(pal_radii[i] > max_val) {
      max_val = pal_radii[i];
      max_index = i;
    }
  }
  
  size_t start_pos = max_index / 2 - max_val;
  size_t pal_size = 0;
  if(max_index % 2 == 1) {
    pal_size = 2 * max_val + 1;
  } else {
    pal_size = 2 * max_val;
  }

  char* longest = calloc(1, pal_size * sizeof(char) + 1);
  strncpy(longest, query_string + start_pos, pal_size);

  return longest;
}

/*
 * Print a string and its palindrome radii.
 */
void print_radii_and_query(char* query_str, size_t query_length, size_t* radii)
{

  size_t i = 0;
  for(i = 0; i < 2 * query_length + 1; i++) {
    if(i % 2 == 1) printf("  %c", query_str[i/2]);
    if(i % 2 == 0) printf("   ");
  } 
  printf("\n");
  for(i = 0; i < 2 * query_length + 1; i++) {
    if(radii[i] < 10) printf(" ");
    printf(" %zd", radii[i]);
  }
  printf("\n");
}

