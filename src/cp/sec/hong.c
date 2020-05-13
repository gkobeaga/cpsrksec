#include "../../cpsrksec.h"

static int
test_cut(struct graph *graph, struct repo *repo, int cvcount,
         struct vertex **cverts, double val);

int
cp_sec_hong(struct cp_par *par, struct graph *graph, struct repo *repo)
{
  int rval = 0;
  int oldn3v;

  int reorder;

  double val;

  struct vertex *squeue = NULL;
  struct arc *f;
  struct vertex **cverts = NULL;
  struct vertex *other, *currvert, *nextvert;
  int cvcount;

  nextvert = graph->tail->next;
  while (nextvert)
  {
    currvert           = nextvert;
    nextvert->mark_aux = 0;

    rval = mincut_solve(graph, graph->tail, currvert, &val, &cverts, &cvcount);
    check_rval(rval, "solve_mincut failed", CLEANUP);

    test_cut(graph, repo, cvcount, cverts, val);
    free(cverts);

    /* Shrink vertices if Rule S3 is considered */
    if (par->srk_s3)
    {
      reorder = 0;

      f = graph_find_arc(graph, graph->tail, currvert);
      if (f && f->x > graph->tail->y)
        reorder = 1;

      graph_identify_vertices(graph, graph->tail, currvert);
      check_rval(rval, "identify_srkvertices failed", CLEANUP);

      if (graph->n3v > 1 && graph->na > 1)
      {
        if (reorder)
          graph_reorder_vertices(graph);
        if (par->srk_extra)
        {
          squeue = NULL;
          for (f = graph->tail->edge; f; f = outnext(f, graph->tail))
          {
            other        = otherend(f, graph->tail);
            other->qnext = squeue;
            squeue       = other;
          }
          graph->tail->qnext = squeue;
          squeue             = graph->tail;

          /****************************************************/

          oldn3v = graph->n3v;
          if (par->srk_rule != CP_SRK_NONE && graph->na > 3)
          {
            rval = cp_shrink_graph(par, graph, squeue, repo);
            check_rval(rval, "shrink_cp_c1 failed", CLEANUP);
          }
          par->count_extra += oldn3v - graph->n3v;
        }

        nextvert = graph->tail->next;
        while (nextvert && !(nextvert->mark_aux)) nextvert = nextvert->next;
      }
      else
      {
        nextvert = NULL;
      }
    }
    else
    {
      nextvert = graph->tail->next;
      while (nextvert && !(nextvert->mark_aux)) nextvert = nextvert->next;
    }
  }

CLEANUP:

  return rval;
}

static int
test_cut(struct graph *graph, struct repo *repo, int cvcount,
         struct vertex **cverts, double val)
{
  int rval = 0;
  int i;
  struct clique *clique = NULL;
  struct vertex *v;
  struct vertex **verts = NULL;
  int nverts;
  double maxweight = 0.0;

  graph->marker++;

  verts = malloc(graph->nv * sizeof(struct vertex *));
  check_null(verts, "out of memory", CLEANUP);

  if (val < 2.0 - ZEROPLUS)
  {
    for (i = 0, nverts = 0; i < cvcount; i++)
    {
      verts[nverts++] = cverts[i]->orig;
#if 1
      if (maxweight < cverts[i]->orig->y)
        maxweight = cverts[i]->orig->y;
      for (v = cverts[i]->members; v; v = v->members)
      {
        if (maxweight < v->orig->y)
          maxweight = v->orig->y;
        verts[nverts++] = v->orig;
      }
#else
      if (maxweight < cverts[i]->max->orig->y)
        maxweight = cverts[i]->max->orig->y;
#endif
    }

    if (val - 2 * maxweight < 0 && nverts > 2 && nverts < graph->nv - 2)
    {
      if (nverts <= graph->nv / 2)
        clique = conv_vertices2clique(graph->orig, verts, nverts);
      else
        clique = conv_vertices2coclique(graph->orig, verts, nverts);
      clique->val = val;
      repo_clique_register(graph->orig, repo, clique);
      clique_free(clique);
    }
  }
  if (verts)
    free(verts);
CLEANUP:
  return rval;
}
