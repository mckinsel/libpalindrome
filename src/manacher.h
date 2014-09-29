#ifndef _manacher_H_
#define _manacher_H_

size_t* manacher(char* query_string, size_t query_length);

int verify_palindrome_radii(char* query_string, size_t query_length, size_t* pal_radii);

char* longest_palindrome(char* query_string, size_t query_length, size_t* pal_radii);

void print_radii_and_query(char* query_str, size_t query_length, size_t radii);

#endif
