#include "minunit.h"
#include "test_utils.h"
#include "manacher/manacher.h"

char* test_panama()
{
  char str[] = "AMANAPLANACANALPANAMA";
  size_t str_len = sizeof(str) - 1;
  size_t* radii = manacher(str, str_len);

  int rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 0, "Faild radii verification for panama palindrome.");
  
  mu_assert(radii[21] == 10, "Failed to find full AMANAPLANACANALPANAMA palindrome.");

  print_radii_and_query(str, str_len, radii);

  free(radii);
  return NULL;
}

char* test_run()
{
  char str[] = "TTTTTTTTT";
  size_t str_len = sizeof(str) - 1;
  size_t* radii = manacher(str, str_len);
  
  int rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 0, "Failed radii verification for run of Ts palindrome.");

  free(radii);
  return NULL;
}

char* test_empty()
{
  char str[] = "";
  size_t str_len = sizeof(str) - 1;
  size_t* radii = manacher(str, str_len);
   
  int rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 0, "Failed radii verification for empty string.");

  free(radii);
  return NULL;
}

char* test_verification()
{
  char str[] = "ACBBCAAAA";
  size_t str_len = sizeof(str) - 1;
                 /* 0  1  2  3  4  5  6  7  8  9 */
  size_t radii[] = {0, 0, 0, 0, 0, 0, 3, 0, 0, 0,
                    0, 0, 1, 1, 2, 1, 1, 0, 0};
  
  int rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 0, "Failed verification for correct radii");

  radii[0] = 1;
  fprintf(stderr, "Expect a warning about a non-zero first element here:\n");
  rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 1, "Verification didn't fail on non-zero first element.");

  radii[0] = 0;
  radii[18] = 1;
  fprintf(stderr, "Expect a warning about a non-zero last element here:\n");
  rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 1, "Verification didn't fail on non-zero last element.");

  radii[18] = 0;
  radii[6] = 2;
  fprintf(stderr, "Expect a warning about a non-maximal palindrome here:\n");
  rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 1, "Verification didn't fail on non-maximal palindrome");

  radii[6] = 3;
  radii[13] = 2;
  fprintf(stderr, "Expect a warning about a failed char comparison here:\n");
  rc = verify_palindrome_radii(str, str_len, radii);
  mu_assert(rc == 1, "Verification didn't fail on incorrect palindrome");
    
  return NULL;
}


char* test_random_strings()
{
  const size_t str_len = 500000;
  char* str = malloc(str_len * sizeof(char));
  unsigned int i = 0;

  printf("Some palindromes:\n");

  for(i = 0; i < 25; i++) {
    random_string(str, str_len);
    size_t* radii = manacher(str, str_len);
    int rc = verify_palindrome_radii(str, str_len, radii);
    mu_assert(rc == 0, "Failed test of random string.");

    char* longest = longest_palindrome(str, str_len, radii);
    printf("%s\n", longest);

    free(longest);
    free(radii);
  }
  printf("\n");
  free(str);

  return NULL;
}
    
char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_panama);
  mu_run_test(test_run);
  mu_run_test(test_empty);
  mu_run_test(test_verification);
  mu_run_test(test_random_strings);

  return NULL;
}

RUN_TESTS(all_tests);
