#include "minunit.h"
#include "test_utils.h"
#include "suffix_tree/suffix_tree.h"
#include "kolpakov_kucherov/equivalence_class_private.h"

/*
 * Test substring class assignment for the string MISSISSIPPI.
 */
char* test_substr_class_mississippi()
{
  char str[] = "MISSISSIPPI";
  size_t str_len = sizeof(str) - 1;
  SuffixTree_T stree = SuffixTree_create(str, str_len);
  size_t* substr_classes = annotate_substr_classes(str_len, 3, stree); 
  
  mu_assert(substr_classes[1] == substr_classes[4],
            "Two ISS substrings have different class ids.");
  mu_assert(substr_classes[2] == substr_classes[5],
            "Two SIS substrings have different class ids.");
  mu_assert(substr_classes[3] != substr_classes[6],
            "Substring SIS and SIP have the same class id.");

  int ret = verify_substr_classes(str, str_len, 3, substr_classes);
  mu_assert(ret == 0, "Verification of MISSISSIPPI substring classes failed.");

  SuffixTree_delete(&stree);
  free(substr_classes);
  return NULL;
}

/*
 * Test substring class assignment for random strings.
 */
char* test_random_strings()
{

  const size_t str_len = 5000;
  char* str = calloc((str_len + 1), sizeof(char));
  size_t substr_len;

  unsigned int i = 0;
  int ret = 0;
  for(i = 0; i < 10; i++) {
    random_string(str, str_len);
    SuffixTree_T stree = SuffixTree_create(str, str_len);
    substr_len = rand() % 100;
    size_t* substr_classes = annotate_substr_classes(str_len, substr_len, stree);

    ret = verify_substr_classes(str, str_len, substr_len, substr_classes);
    mu_assert(ret == 0, "Verification of random string substring classes failed.");

    SuffixTree_delete(&stree);
    free(substr_classes);
  }
  free(str);

  return NULL;
}

/*
 * Test that the substring class verification function fails when the substr
 * classes are incorrect.
 */
char* test_substr_class_verification()
{
  char str[] = "BANANA";
  size_t str_len = sizeof(str) - 1;
  SuffixTree_T stree = SuffixTree_create(str, str_len);
  size_t* substr_classes = annotate_substr_classes(str_len, 3, stree);
  
  /* Should be something like
   *    B A N A N A   string
   *    1 2 3 2 0 0   classes
   *    0 1 2 3 4 5   index
   */
  
  size_t i = 0, j = 0;
  for(i = 0; i < str_len; i++) {
    for(j = i; j < str_len; j++) {
      if(i == j) continue;
      if(i > 3) {
        mu_assert(substr_classes[i] == 0, "Failed assignment of zero class.");
        continue;
      }

      if(i == 1 && j == 3) {
        mu_assert(substr_classes[i] == substr_classes[j],
                  "Failed assignment of same class ids.");
      } else {
        mu_assert(substr_classes[i] != substr_classes[j],
                  "Failed assignment of different class ids.");
      }
    }
  }

  /* Now we know that substr_classes is correct. */
  int ret = verify_substr_classes(str, str_len, 3, substr_classes);
  mu_assert(ret == 0, "Verification marked correct substring classes as incorrect.");

  size_t* bad_classes = malloc(str_len * sizeof(size_t));

  memcpy(bad_classes, substr_classes, str_len*sizeof(size_t));
  bad_classes[1] = 99;
  fprintf(stderr, "Expect substr_class warning:\n");
  ret = verify_substr_classes(str, str_len, 3, bad_classes);
  mu_assert(ret == 1, "Verification marked incorrect substring classes as correct.");

  memcpy(bad_classes, substr_classes, str_len);
  bad_classes[5] = 2;
  fprintf(stderr, "Expect substr_class warning:\n");
  ret = verify_substr_classes(str, str_len, 3, bad_classes);
  mu_assert(ret == 1, "Verification marked incorrect substring classes as correct.");

  memcpy(bad_classes, substr_classes, str_len);
  bad_classes[0] = bad_classes[2];
  fprintf(stderr, "Expect substr_class warning:\n");
  ret = verify_substr_classes(str, str_len, 3, bad_classes);
  mu_assert(ret == 1, "Verification marked incorrect substring classes as correct.");
  
  SuffixTree_delete(&stree);
  free(substr_classes);
  free(bad_classes);

  return NULL;
}

char* all_tests()
{
  mu_suite_start();
  
  mu_run_test(test_substr_class_mississippi);
  mu_run_test(test_random_strings);
  mu_run_test(test_substr_class_verification);

  return NULL;
}

RUN_TESTS(all_tests);
