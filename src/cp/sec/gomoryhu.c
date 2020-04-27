#include "../../cpsrksec.h"

static int
test_cuts(struct graph *graph, struct ghnode *parent, struct repo *repo);
static void
get_verts(struct ghnode *parent, int *count, struct vertex **verts);

int
cp_sec_gomoryhu(struct cp_par *par, struct graph *graph, struct repo *repo)
{
  int rval = 0;
  struct ghnode *child;

  struct ghtree *ghtree;

  ghtree_get(par, graph, graph->tail->i, &ghtree);

  for (child = ghtree->root; child; child = child->next_sibling)
    test_cuts(graph, child, repo);

  ghtree_free(ghtree);

  return rval;
}

static int
test_cuts(struct graph *graph, struct ghnode *parent, struct repo *repo)
{
  int rval = 0;
  int i;
  struct clique *clique  = NULL;
  struct vertex **cverts = NULL;
  struct vertex **verts  = NULL;
  struct vertex *v;
  int nverts;
  int cvcount = 0;
  struct ghnode *child;

  if (parent->cutval - 2 * parent->in_max->y < 0 - ZEROPLUS && parent->ndesc > 1)
  {
    verts = malloc(graph->nv * sizeof(struct vertex *));
    check_null(verts, "out of memory", CLEANUP);
    cverts = malloc(parent->ndesc * sizeof(struct vertex *));
    check_null(cverts, "out of memory", CLEANUP);

    cvcount = 0;
    get_verts(parent, &cvcount, cverts);
    assert(cvcount == parent->ndesc);

    for (i = 0, nverts = 0; i < cvcount; i++)
    {
      verts[nverts++] = cverts[i];
      for (v = cverts[i]->shrunk->members; v; v = v->members)
        verts[nverts++] = v->orig;
    }

    if (nverts <= graph->nv / 2 && nverts > 2 && nverts < graph->nv - 2)
      clique = conv_vertices2clique(graph, verts, nverts);
    else
      clique = conv_vertices2coclique(graph, verts, nverts);
    clique->val = parent->cutval;

    repo_clique_register(graph, repo, clique);

    clique_free(clique);
    free(verts);
    free(cverts);
  }

  for (child = parent->child; child; child = child->next_sibling)
  {
    test_cuts(graph, child, repo);
  }

CLEANUP:
  return rval;
}

static void
get_verts(struct ghnode *parent, int *count, struct vertex **verts)
{
  struct ghnode *child;

  verts[(*count)++] = parent->special->orig;

  for (child = parent->child; child; child = child->next_sibling)
  {
    get_verts(child, count, verts);
  }

  return;
}
