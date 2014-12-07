[![Build
Status](https://travis-ci.org/mckinsel/libpalindrome.svg?branch=master)](https://travis-ci.org/mckinsel/libpalindrome)

# libpalindrome
libpalindrome provides functions for finding palindromes in strings. Classically,
a palindrome is a sequence that reads the same forward and backward. For
example, the string MADAMIMADAM is a palindrome. There are variations on the
definition of palindrome:

1. Gapped palindromes. These palindromes take the form uvu<sup>T</sup>, where
   u<sup>T</sup> is the reverse of u, and v can be any string. For example,
   AHSATANSEZWXOGVWSXGESNATASHA is a gapped palindrome. u = AHSATANSE, v =
   ZWXOGVWSXG, and u<sup>T</sup>=ESNATASHA.

2. Approximate palindromes. These palindromes are nearly the same forward and
   backward. More precisely, they are a string such that the Levenshtein
   distance between the string and its reverse is less than some bound. For
   example, consider a modification of the string above, MADAMIMALAN. The
   Levenshtein distance between this string and its reverse is 4, so if we set
   the distance bound to at least 4, this is an approximate palindrome.

3. Alternative definitions of equality. In some problem domains, we are not
   interested in strings whose forward and reverse are indentical. For example,
   when identifying palindromes in a string of DNA, we want a string that is
   identical to its reverse complement. For example, the string ACCTAGGT is its
   own reverse complement.
