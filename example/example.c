#include "cpsrksec.h"

#define RNG_SEED (22)

#define V_COUNT (9)
#define E_COUNT (15)
static int end_verts[2 * E_COUNT] = {
/* Edge values */
1, 2,
1, 7,
1, 9,
2, 3,
2, 6,
2, 8,
3, 4,
3, 5,
3, 8,
4, 5,
5, 6,
5, 8,
6, 7,
6, 9,
7, 9,
};
static double lp_x[V_COUNT + E_COUNT] = {
/* Vertex values */
0.750, /* y_1 */
1.000, /* y_2 */
0.750, /* y_3 */
0.125, /* y_4 */
0.750, /* y_5 */
1.000, /* y_6 */
0.750, /* y_7 */
0.750, /* y_8 */
0.750, /* y_9 */
/* Edge values */
0.500, /* x_{1,2} */
0.500, /* x_{1,7} */
0.500, /* x_{1,9} */
0.750, /* x_{2,3} */
0.250, /* x_{2,6} */
0.500, /* x_{2,8} */
0.125, /* x_{3,4} */
0.125, /* x_{3,5} */
0.500, /* x_{3,8} */
0.125, /* x_{4,5} */
0.750, /* x_{5,6} */
0.500, /* x_{5,8} */
0.500, /* x_{6,7} */
0.500, /* x_{6,9} */
0.500, /* x_{7,9} */
};

int
main(int argc, char *argv[])
{
  int rval = 0;
  struct cp_par *par;

  struct graph *supportgraph = NULL;
  struct graph *srkgraph     = NULL;
  struct arc *arc;

  struct vertex **verts = NULL;

  int lp_ind;

  int cutcount       = 0;
  struct cpcut *cuts = NULL;
  struct cpcut *cut;

  struct vertex **nodevector = NULL;
  struct repo *repo          = NULL;

  par = cp_create_params();

  srand (time(NULL));
#ifdef RNG_SEED
  rng_init_rand(par->rand, RNG_SEED);
#else
  rng_init_rand(par->rand, rand());
#endif

  supportgraph = graph_create();
  check_null(supportgraph, "graph_create failed", CLEANUP);

  /* Add the vertices in the cycle problem to the support graph */
  graph_add_vertices(supportgraph, V_COUNT);

  /* If a vertex must be visited in the solution of the cycle problem,
   * e.j. a cycle problem with depot, you can mark as fixed.
   * Multiple fixed vertices are possible. */
   //supportgraph->v[1]->fixed = 1;

  /* Assuming the degree constraints are included in the LP_0 */

  for (lp_ind = V_COUNT; lp_ind < V_COUNT + E_COUNT; lp_ind++)
  {
    if (lp_x[lp_ind] > 0.0)
    {
      /* Note that, in C the array indices start from 0 */
      arc      = graph_add_arc(supportgraph,
                               end_verts[2 * (lp_ind - V_COUNT)] - 1,
                               end_verts[2 * (lp_ind - V_COUNT) + 1] - 1);
      arc->x   = lp_x[lp_ind];
      arc->tail->y += arc->x / 2.0;
      arc->head->y += arc->x / 2.0;

      // The index of arc in support graph might not be same as index in the lp
      arc->ind = lp_ind;
    }
  }

  /* This verfies that the example is defined as expected.
   * It is not needed in practice. */
  for (lp_ind = 0; lp_ind < V_COUNT; lp_ind++)
  {
    assert(supportgraph->v[lp_ind]->y == lp_x[lp_ind]);
  }

  repo = repo_clique_create(supportgraph);

  srkgraph = supportgraph->shrunk = graph_create();

  rval = graph_copy(supportgraph, srkgraph);
  check_rval(rval, "graph_copy failed", CLEANUP);

  graph_reorder_vertices(srkgraph);

  /* 1. Pre-process: shrinking */
  //par->srk_rule = CP_SRK_NONE;
  par->srk_s2   = 0;
  if (par->srk_rule != CP_SRK_NONE)
  {
    rval = cp_shrink_graph(par, srkgraph, NULL, repo);
    check_rval(rval, "cp_shrink_graph failed", CLEANUP);
  }

  /* 2. SEC separation */
  par->sec_sep = CP_SEC_GOMORYHU;
  //par->sec_sep   = CP_SEC_HONG;
  // par->srk_s3    = 1;
  // par->srk_extra = 0;
  cp_sec_sep(par, srkgraph, repo);

  /* 3. Cut generation */
  repo_clique_get_cuts(par, supportgraph, repo, &cutcount, &cuts);

  /* 4. Add to the LP */
  while (cuts)
  {
    cut  = cuts;
    cuts = cut->next;

    cp_print_cut(cut);

    cp_free_cut(&cut);
  }

CLEANUP:
  if (repo)
    repo_clique_free(&repo);
  if (nodevector)
    free(nodevector);
  if (verts)
    free(verts);
  graph_free(supportgraph);
  cp_free_params(&par);

  return rval;
}
