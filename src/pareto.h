/* pareto.h -- Defines for the pareto front code.
 */


#ifndef pareto_h
#define pareto_h


#define MAX_DIMENSION 5

typedef struct
{
  float v[MAX_DIMENSION];
  int rank;
  int side; // Used for the marriage step
  int tiebreaker; // Used for getting stable sorting
  int valid; // In k-dimensional, we only count points
  int id;
  // that are in the A subset -- in the projection, we want
  // to ignore counting Bs
} pareto_point;

typedef struct
{
  int num_points;  // Number of points in a pareto problem
  int num_dimensions; // Total dimension
  pareto_point **p;   // An array of *pointers* to the original points
  pareto_point *points; // If we need to allocate points and clone
} pareto_problem;

int ecdfk(pareto_problem *problem);
void bruteforce_rank(pareto_problem *prob);

#endif

