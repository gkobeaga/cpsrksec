#include "../../cpsrksec.h"

int
cp_shrink_graph(struct cp_par *par, struct graph *graph, struct vertex *qstart, struct repo *repo)
{
  int rval = 0;

  switch (par->srk_rule)
  {
  case CP_SRK_C1:
    cp_shrink_c1(par, graph, qstart, repo);
    break;
  case CP_SRK_C1C2:
    cp_shrink_c1c2(par, graph, qstart, repo);
    break;
  case CP_SRK_C1C2C3:
    cp_shrink_c1c2c3(par, graph, qstart, repo);
    break;
  case CP_SRK_S1:
    cp_shrink_s1(par, graph, qstart, repo);
    break;
  default:
    rval = 1;
    fprintf(stderr, "Invalid shrinking rule in shrink_cp_graph\n");
    break;
  }

  return rval;
}

int
repo_cp_save_vertices(struct graph *graph, struct vertex *u, struct vertex *v,
                      double eweight, struct repo *repo)
{
  int rval = 0;

  int i, t1vcount, t2vcount, cvcount;
  struct vertex **t1verts=NULL, **t2verts=NULL, **cverts=NULL;
  struct clique *clique = NULL;

  rval = graph_expand_vertex(graph, u, &t1vcount, &t1verts);
  check_rval(rval, "graph_expand_vertex failed", CLEANUP);
  rval = graph_expand_vertex(graph, v, &t2vcount, &t2verts);
  check_rval(rval, "graph_expand_vertex failed", CLEANUP);

  cvcount = t1vcount + t2vcount;
  cverts  = malloc(cvcount * sizeof(struct vertex *));

  for (i = 0; i < t1vcount; i++) cverts[i] = t1verts[i];
  for (i = 0; i < t2vcount; i++) cverts[t1vcount + i] = t2verts[i];

  if (cvcount <= graph->nv / 2 && cvcount > 2 && cvcount < graph->nv - 2)
    clique = conv_vertices2clique(graph->orig, cverts, cvcount);
  else
    clique = conv_vertices2coclique(graph->orig, cverts, cvcount);
  clique->val = 2*u->y + 2*v->y - 2 * eweight;
  repo_clique_register(graph->orig, repo, clique);

CLEANUP:
  if (clique)
    clique_free(clique);
  if (t1verts)
    free(t1verts);
  if (t2verts)
    free(t2verts);
  if (cverts)
    free(cverts);
  return rval;
}
