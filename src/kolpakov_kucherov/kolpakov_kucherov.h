#ifndef _kolpakov_kucherov_H_
#define _kolpakov_kucherov_H_

#include <stdlib.h>


void length_constrained_palindromes(char* query_string, size_t query_length,
                                    size_t min_arm_length, size_t min_gap_length,
                                    size_t max_gap_length);

#endif
