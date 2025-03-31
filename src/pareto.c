/* pareto.c -- Code to find a multidimensional Pareto front,
 * using Bentley's Multidimensional Divide-and-Conquer approach.
 *
 * Input will be an array of points (each with an array of values
 * for each axis), and the number of dimensions.
 *
 * Each point will get a rank value -- using the ECDF/Maxima
 * hack, you can feed in the negative values on the axis, look for
 * the total number of points dominated, and then any point with
 * a zero-dominance value in the negative ECDF problem is a
 * maxima/Pareto-point in the original problem.
 *
 * Pretty slick stuff.  Gonna need a lot of coffee for this one.
 */
#include <stdio.h>
#include <stdlib.h>
#include "pareto.h"

#define PARETODEBUG 0

static int compare_index;
static int par_point_compare(const void *a, const void *b)
{
    pareto_point *point_a, *point_b;
    
    point_a = *((pareto_point **)a);
    point_b = *((pareto_point **)b);
    
    if (point_a->v[compare_index] > point_b->v[compare_index])
        return 1;
    if (point_a->v[compare_index] < point_b->v[compare_index])
        return -1;
    if (point_a->tiebreaker > point_b->tiebreaker)
        return 1;
    if (point_a->tiebreaker < point_b->tiebreaker)
        return -1;
    
    return 0;
}

static void pareto_showpoint(pareto_problem *prob, int point_num)
{
    int k;
    pareto_point *pt = prob->p[point_num];
    
    printf("[%3d  r%3d (", pt->tiebreaker, pt->rank);
    for (k = 0; k < prob->num_dimensions; ++k)
    {
        printf(" %5.1f", pt->v[k]);
    }
    printf(")]  s%d v%d\n", pt->side, pt->valid);
}


static void pareto_print(pareto_problem *prob)
{
    int n, i;
    
    for (n = 0; n < prob->num_points; ++n)
    {
        pareto_showpoint(prob, n);
    }
}

static int pareto_sort(pareto_problem *problem, int index)
{
    compare_index = index;
    qsort(problem->p, problem->num_points,
          sizeof(pareto_point *), par_point_compare);
    
    return 0;
}


int ecdf(pareto_problem *problem)
{
    int i, k, acount;
    
    if (PARETODEBUG)
    {
        printf("Pareto problem with %d points\n", problem->num_points);
        pareto_print(problem);
    }
    if (problem->num_dimensions <= 0)
        return 0;
    
    if (problem->num_points <= 1)
    {
        if (problem->num_points == 1)
        {
            // Fix the rank to be 0;
            problem->p[0]->rank = 0;
        }
        return 0;
    }
    
    // Make sure that we're sorted by X
    pareto_sort(problem, 0);
    
    // Split this problem into two subproblems, A and B
    pareto_problem a, b;
    a.p = problem->p;
    a.num_dimensions = problem->num_dimensions;
    int mid = problem->num_points/2;
    a.num_points = mid;
    b.p = &problem->p[mid];
    b.num_dimensions = problem->num_dimensions;
    b.num_points = problem->num_points - mid;
    
    if (PARETODEBUG)
        printf("Split problem %d points %d dim into problems with %d and %d\n",
               problem->num_points,
               problem->num_dimensions,
               a.num_points, b.num_points);
    ecdf(&a);
    ecdf(&b);
    
    // Marriage step.
    for (i = 0; i < mid; ++i)
        problem->p[i]->side = 0;
    for (i = mid; i < problem->num_points; ++i)
        problem->p[i]->side = 1;
    
    pareto_sort(problem, 1);
    
    acount = 0;
    if (PARETODEBUG)
    {
        printf("Marriage step.\n");
        pareto_print(problem);
        printf("Looking to merge.\n");
    }
    for (i = 0; i < problem->num_points; ++i)
    {
        if (PARETODEBUG)
            pareto_showpoint(problem, i);
        
        if (problem->p[i]->side == 0)
        {
            acount += problem->p[i]->valid;
            if (PARETODEBUG)
                printf("Acount incremented to %d\n", acount);
        }
        else
        {
            if (PARETODEBUG)
                printf("Add %d to point %d\n",
                       acount,
                       problem->p[i]->tiebreaker);
            problem->p[i]->rank += acount;
        }
    }
    
    return 0;
}




int ecdfk(pareto_problem *problem)
{
    int i, k, acount;
    
    if (PARETODEBUG)
    {
        printf("Pareto problem with %d points\n", problem->num_points);
        pareto_print(problem);
    }
    
    if (problem->num_points <= 1)
    {
        if (problem->num_points == 1)
        {
            // Fix the rank to be 0;
            problem->p[0]->rank = 0;
        }
        return 0;
    }
    
    if (problem->num_dimensions < 2)
        return 0;
    
    if (problem->num_dimensions == 2)
        return ecdf(problem);
    
    // Ok, we've got a multidimensional problem.
    // Sort along the last plane, so that we can split
    // into two subproblems.
    pareto_sort(problem, problem->num_dimensions - 1);
    
    // Split this problem into two subproblems, A and B
    pareto_problem a, b;
    a.p = problem->p;
    a.num_dimensions = problem->num_dimensions;
    int mid = problem->num_points/2;
    a.num_points = mid;
    b.p = &problem->p[mid];
    b.num_dimensions = problem->num_dimensions;
    b.num_points = problem->num_points - mid;
    
    if (PARETODEBUG)
        printf("Split problem %d points %d dim into problems with %d and %d\n",
               problem->num_points,
               problem->num_dimensions,
               a.num_points, b.num_points);
    ecdfk(&a);
    ecdfk(&b);
    
    // Now build a NEW problem -- with one less dimension
    pareto_problem projection;
    projection.points = (pareto_point *)malloc(problem->num_points * sizeof(pareto_point));
    projection.p = (pareto_point **)malloc(problem->num_points * sizeof(pareto_point *));
    pareto_sort(problem, problem->num_dimensions - 1);
    for (i = 0; i < problem->num_points; ++i)
    {
        projection.p[i] = &projection.points[i];
        projection.points[i] = *problem->p[i];
        projection.points[i].rank = 0;
        if ((i < mid) && (problem->p[i]->valid))
            projection.points[i].valid = 1;
        else
            projection.points[i].valid = 0;
    }
    
    projection.num_dimensions = problem->num_dimensions - 1;
    projection.num_points = problem->num_points;
    
    
    if (PARETODEBUG)
        printf("Now handling the projection\n");
    
    // Sort by X
    pareto_sort(&projection, 0);
    ecdfk(&projection);
    
    // Get back to the original sorting, so that we can recover
    // the A and B sides
    pareto_sort(problem, problem->num_dimensions - 1);
    pareto_sort(&projection, problem->num_dimensions - 1);
    
    // Marriage step.
    for (i = 0; i < mid; ++i)
    {
        problem->p[i]->side = 0;
        projection.p[i]->side = 0;
    }
    for (i = mid; i < problem->num_points; ++i)
    {
        problem->p[i]->side = 1;
        projection.p[i]->side = 1;
    }
    
    acount = 0;
    if (PARETODEBUG)
    {
        printf("***PROJECTIONS Marriage step.\n");
        printf("A:\n");
        pareto_print(&a);
        printf("B:\n");
        pareto_print(&b);
        printf("Projection:\n");
        pareto_print(&projection);
        printf("\nStarting to merge.\n");
    }
    for (i = 0; i < problem->num_points; ++i)
    {
        if (PARETODEBUG)
            pareto_showpoint(problem, i);
        if (problem->p[i]->tiebreaker != projection.p[i]->tiebreaker)
        {
            printf("ERROR IN THE MERGE!  Tiebreaker match is wrong.\n");
        }
        
        if (problem->p[i]->side == 0)
        {
            // Ignore anything on side A
        }
        else
        {
            int result = projection.p[i]->rank + problem->p[i]->rank;
            if (PARETODEBUG)
                printf("Point %d has %d in B, %d in projection, result %d\n",
                       projection.p[i]->tiebreaker,
                       problem->p[i]->rank,
                       projection.p[i]->rank,
                       result);
            problem->p[i]->rank = result;
        }
    }
    
    free(projection.p);
    free(projection.points);
    
    return 0;
}


int dominates(pareto_problem *prob, int i, int j)
{
    int k;
    pareto_point *a, *b;
    
    a = prob->p[i];
    b = prob->p[j];
    
    for (k = 0; k < prob->num_dimensions; ++k)
    {
        if (a->v[k] < b->v[k])
            return 0;
        if ((a->v[k] == b->v[k]) &&
            (a->tiebreaker < b->tiebreaker))
            return 0;
    }
    return 1;
}


void bruteforce_rank(pareto_problem *prob)
{
    int i, j;
    
    for (i = 0; i < prob->num_points; ++i)
        prob->p[i]->rank = 0;
    
    for (i = 0; i < prob->num_points; ++i)
        for (j = 0; j < prob->num_points; ++j)
        {
            if (i == j)
                continue;
            
            if (dominates(prob, i, j))
                ++prob->p[i]->rank;
        }
    
}

void pareto_hello() {
  printf("C Pareto code.\n");
}

static pareto_problem *stat_problem = NULL;
static pareto_point *stat_points = NULL;

void pareto_set_size(int num_points, int num_dimensions) {
    // printf("Create a Pareto problem in C with %d points, %d dimensions\n", num_points, num_dimensions);
    stat_problem = (pareto_problem *)malloc(sizeof(pareto_problem));
    stat_problem->points = (pareto_point *)malloc(sizeof(pareto_point) * num_points);
    stat_problem->p = (pareto_point **)malloc(sizeof(pareto_point *) * num_points);
    for (int i = 0; i < num_points; ++i)
        stat_problem->p[i] = &stat_problem->points[i];

    stat_problem->num_points = num_points;
    stat_problem->num_dimensions = num_dimensions;
}
void pareto_set_nv(int pn, int dim, float v) {
    // printf("Set point %d dimension %d to %f\n", pn, dim, v);
    stat_problem->points[pn].v[dim] = v;
    stat_problem->points[pn].id = pn;
    stat_problem->points[pn].tiebreaker = pn;
    stat_problem->points[pn].valid = 1;
}
void pareto_set_id(int pn, int tag) {
    stat_problem->points[pn].id = tag;
}
// Might need to do some more here.  Depending on tie
// breaking, some things that should get dominated
// are interpreted as non-dominated.  Ties on the
// other axis can make life hard, need a good consistent
// way to break the ties.
void pareto_generate_tiebreak(){
    for (int i = 0; i < stat_problem->num_points; ++i)
    {
        float v = 0;
        for (int d = 0; d < stat_problem->num_dimensions; ++d) {
            v = v - stat_problem->points[i].v[d];
        }
        stat_problem->points[i].tiebreaker = (int) v + i;
    }
}

void pareto_free() {
    // Gets rid of a pareto problem.
    if (stat_problem)
    {
        free(stat_problem->p);
        free(stat_problem->points);
        free(stat_problem);
    }
    stat_problem = NULL;
}
void pareto_solve() {
    // printf("Stat problem is at %p\n", stat_problem);
    // pareto_print(stat_problem);
    // bruteforce_rank(stat_problem);
    ecdfk(stat_problem);
    if (PARETODEBUG)
        pareto_print(stat_problem);
}
int pareto_get_rank(int pn) {
    pareto_point *pt = stat_problem->p[pn];
    return pt->rank;
}
int pareto_get_id(int pn) {
    pareto_point *pt = stat_problem->p[pn];
    return pt->id;
}


#if 0

int main()
{
    int i, k, n;
    int ik;
    pareto_point points[1000];
    pareto_problem prob;
    
    scanf("%d %d", &n, &k);
    for (i = 0; i < n; ++i)
    {
        points[i].rank = 0;
        points[i].tiebreaker = i;
        for (ik = 0; ik < k; ++ik)
            scanf("%f", &points[i].v[ik]);
        points[i].valid = 1;
    }
    
    
    printf("%d points, %d axis\n", n, k);
    prob.p = (pareto_point **)malloc(n * sizeof(pareto_point *));
    prob.num_points = n;
    prob.num_dimensions = k;
    for (i = 0; i < n; ++i)
    {
        prob.p[i] = &points[i];
    }
    
#if 0
    // Make sure the sort works.
    for (ik = 0; ik < k; ++ik)
    {
        printf("Sort by index %d\n", ik);
        compare_index = ik;
        qsort(prob.p, n, sizeof(pareto_point *), par_point_compare);
        pareto_print(&prob);
    }
#endif
    
    // Sort by X axis.  We only need to do this once.
    // We'll be splitting on other axis later, and because
    // of the recursive nature, we don't need to revise
    // the order (it's sort of like merge sort, kind of -- or maybe
    // quicksort -- subregions may get shuffled, but it's all ok.)
    pareto_sort(&prob, 0);
    
    if (prob.num_dimensions > 2)
        ecdfk(&prob);
    else
        ecdf(&prob);
    
    printf("Final:\n");
    pareto_print(&prob);
    
    printf("Sanity check:\n");
    bruteforce_rank(&prob);
    pareto_print(&prob);
    
}

#endif

