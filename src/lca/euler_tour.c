#include <math.h>
#include <stdlib.h>

#include "dbg.h"
#include "euler_tour.h"

/* A little helper struct for the Euler walk through the suffix tree. */
struct EulerTourWalkData {
  Node_T* nodes;
  SuffixTreeIndex_T* depths;
  size_t* first_instances;
  size_t* pos_in_tour;
};

/* The NodeFunc_T for the walk that will create the EulerTour_T. */
SuffixTreeIndex_T euler_tour_node_func(SuffixTree_T tree,
                                       Node_T node,
                                       void* veuler_data,
                                       SuffixTreeIndex_T depth)
{
  struct EulerTourWalkData* euler_data = veuler_data;
  size_t pos_in_tour = *(euler_data->pos_in_tour);

  euler_data->nodes[pos_in_tour] = node;
  euler_data->depths[pos_in_tour] = depth;

  if(euler_data->first_instances[Node_get_index(node)] == 0) {
    if(SuffixTree_get_root(tree) != node) {
      euler_data->first_instances[Node_get_index(node)] = *(euler_data->pos_in_tour);
    }
  }
  (*euler_data->pos_in_tour)++;
  
  return depth + 1;
}

EulerTour_T EulerTour_create(SuffixTree_T tree)
{
  struct EulerTourWalkData* euler_data = NULL;
  size_t* pos_in_tour = NULL;
  EulerTour_T euler_tour = NULL;

  euler_tour = calloc(1, sizeof(struct EulerTour_T));
  check_mem(euler_tour);

  euler_tour->num_nodes = SuffixTree_get_num_nodes(tree);
  euler_tour->length = 2 * SuffixTree_get_num_nodes(tree) - 1;

  euler_tour->nodes = calloc(euler_tour->length, sizeof(Node_T));
  check_mem(euler_tour->nodes);

  euler_tour->depths = calloc(euler_tour->length, sizeof(SuffixTreeIndex_T));
  check_mem(euler_tour->depths);

  euler_tour->first_instances = calloc(SuffixTree_get_num_nodes(tree),
                                       sizeof(SuffixTreeIndex_T));
  check_mem(euler_tour->first_instances);
  
  pos_in_tour = calloc(1, sizeof(size_t));

  euler_data = malloc(sizeof(struct EulerTourWalkData));
  check_mem(euler_data);

  euler_data->nodes = euler_tour->nodes;
  euler_data->depths = euler_tour->depths;
  euler_data->first_instances = euler_tour->first_instances;
  euler_data->pos_in_tour = pos_in_tour;

  SuffixTree_euler_walk(tree, SuffixTree_get_root(tree),
                        euler_tour_node_func, euler_data,
                        0);

  free(euler_data);
  free(pos_in_tour);

  return euler_tour;

error:
  if(pos_in_tour) free(pos_in_tour);
  if(euler_data) free(euler_data);
  EulerTour_delete(&euler_tour);
  return NULL;
}

void EulerTour_delete(EulerTour_T* euler_tour)
{
  if(!euler_tour) return;

  if(*euler_tour) {
    if((*euler_tour)->nodes) free((*euler_tour)->nodes);
    if((*euler_tour)->depths) free((*euler_tour)->depths);
    if((*euler_tour)->first_instances) free((*euler_tour)->first_instances);
    free(*euler_tour);
  }
}

int EulerTour_verify(EulerTour_T euler_tour, SuffixTree_T tree)
{
  /* First verify that the depths array has the +-1 property */
  size_t i = 0;
  SuffixTreeIndex_T num_nodes = SuffixTree_get_num_nodes(tree);
  Node_T root = SuffixTree_get_root(tree);

  for(i = 0; i < euler_tour->length - 1; i++) {
    long int diff = euler_tour->depths[i] - euler_tour->depths[i + 1];
    
    if(!labs(diff) == 1) {
      log_warn("Consecutive values in depth array do not differ by 1.");
      return 1;
    }
  }

  /* Then check some properties of the tour. The first and last elements should
   * be the root. And, for A, B, C in the tour, if A == C, then B is a leaf in
   * the tree, and the number of leaves is the length of the suffix tree
   * string. */
  if(euler_tour->nodes[0] != root) {
    log_warn("First element in tour is not the root.");
    return 1;
  }
  if(euler_tour->nodes[euler_tour->length - 1] != root) {
    log_warn("Last element in tour is not the root.");
    return 1;
  }
  size_t leaf_count = 0;
  for(i = 0; i < euler_tour->length - 2; i++) {
    if(euler_tour->nodes[i] == euler_tour->nodes[i+2]) leaf_count++;
  }
  if(leaf_count != SuffixTree_get_string_length(tree)) {
    log_warn("Number of leaves in tour, %zu, is not the length of the string, %zu",
            leaf_count, SuffixTree_get_string_length(tree));
    return 1;
  }

  /* Verify that first instances is strictly increasing. This doesn't
   * necessarily have to be true, but since we assign labels in the order that
   * we visit nodes in the same depth-first traversal, it should be true for
   * us. */
  for(i = 0; i < num_nodes - 1; i++) {
    if(euler_tour->first_instances[i+1] <= euler_tour->first_instances[i]){
      log_warn("First instances is not increasing.");
      return 1;
    }
  }

  return 0;
}

void EulerTour_print(EulerTour_T euler_tour)
{
  printf("Euler tour:\n");
  size_t i = 0;
  for(i = 0; i < euler_tour->length; i++) {
    printf("Position: %zu\tNode index: %lu\tDepth: %lu\n",
           i, Node_get_index(euler_tour->nodes[i]),
           euler_tour->depths[i]);
  }

  printf("\nFirst instances:\n");
  for(i = 0; i < euler_tour->num_nodes; i++) {
    printf("Node index: %lu\tFirst instance position: %zu\n",
           i, euler_tour->first_instances[i]);
  }
}
