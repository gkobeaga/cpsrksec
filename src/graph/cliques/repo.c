#include "../../cpsrksec.h"

struct repo *
repo_clique_create(struct graph *graph)
{
  int i;
  struct repo *repo    = malloc(sizeof(struct repo));
  repo->size           = 0;
  repo->cliquespace    = 1000;
  repo->cliquehashsize = 0;
  repo->cliquehash     = NULL;
  repo->cliques        = malloc(repo->cliquespace * sizeof(struct clique *));
  repo->tot_n          = graph->nv;

  repo->cliquehashsize = prime_next((unsigned int)2 * graph->nv);
  repo->cliquehash     = malloc(repo->cliquehashsize * sizeof(int));
  if (!repo->cliquehash)
  {
    repo->cliquehashsize = 0;
    return NULL;
  }
  for (i = 0; i < repo->cliquehashsize; i++)
  {
    repo->cliquehash[i] = -1;
  }
  repo->cliquefree = -1;
  return repo;
}

void
repo_clique_free(struct repo **repo)
{
  int i;
  if (*repo)
  {
    if ((*repo)->cliquehash)
      free((*repo)->cliquehash);
    for (i = 0; i < (*repo)->size; i++)
    {
      clique_free((*repo)->cliques[i]);
    }
    if ((*repo)->cliques)
      free((*repo)->cliques);
    free(*repo);
  }
}

int
repo_clique_register(struct graph *graph, struct repo *repo, struct clique *c)
{
  int x = clique_hash(c) % repo->cliquehashsize;
  int y = repo->cliquehash[x];
  struct clique *clique;

  while (y != -1)
  {
    clique = repo->cliques[y];
    if (clique_eq(c, clique))
    {
      clique->refcount++;
      return y;
    }
    y = clique->hashnext;
  }

  if (repo->cliquefree != -1)
  {
    y                = repo->cliquefree;
    clique           = repo->cliques[y];
    repo->cliquefree = clique->hashnext;
    clique           = repo->cliques[y];
  }
  else
  {
    if (repo->size + 1 >= repo->cliquespace)
    {
      realloc_scale(repo->cliques, repo->cliquespace, repo->size + 1, 1.3);
    }
    y                = repo->size++;
    repo->cliques[y] = clique = malloc(sizeof(struct clique));
  }

  clique_copy(c, clique);

  clique->refcount    = 1;
  clique->hashnext    = repo->cliquehash[x];
  repo->cliquehash[x] = y;

  return y;
}

void
repo_clique_unregister(struct graph *graph, struct repo *repo, int c)
{
  int x, y, yprev;

  repo->cliques[c]->refcount--;
  if (repo->cliques[c]->refcount)
    return;
  x = clique_hash(repo->cliques[c]) % repo->cliquehashsize;
  y = repo->cliquehash[x];
  if (y == c)
  {
    repo->cliquehash[x] = repo->cliques[c]->hashnext;
  }
  else
  {
    yprev = y;
    y     = repo->cliques[y]->hashnext;
    while (y != c && y != -1)
    {
      yprev = y;
      y     = repo->cliques[y]->hashnext;
    }
    if (y == -1)
    {
      fprintf(stderr, "Couldn't find clique to delete from hash\n");
      return;
    }
    repo->cliques[yprev]->hashnext = repo->cliques[c]->hashnext;
  }
  free(repo->cliques[c]->nodes);
  repo->cliques[c]->segcount = -1;
  repo->cliques[c]->hashnext = repo->cliquefree;
  repo->cliquefree           = c;
}

void static choose(struct vertex **dest, int k, struct vertex **src, int n)
{
  int i, j = 0;

  assert(k <= n);

  for (i = 0; i < n && j < k; i++)
  {
    if ((rand() % (n - i)) < k - j)
    {
      dest[j] = src[i];
      j++;
    }
  }
}

int
repo_clique_get_cuts(struct cp_par *par, struct graph *graph, struct repo *repo,
                     int *cutcount, struct cpcut **cuts)
{
  int rval = 0;
  struct clique *clique;
  struct cpcut *cut = NULL;

  int i, j, k;
  int nin = 0, nout = 0;
  int nselin = 0, nselout = 0;
  double cut_val;

  double ymax_in;
  double ymax_out;
  int fixed_in  = 0;
  int fixed_out = 0;

  struct vertex **max_in  = NULL;
  struct vertex **max_out = NULL;
  struct vertex **sel_in  = NULL;
  struct vertex **sel_out = NULL;

  max_in = malloc(graph->nv * sizeof(struct vertex *));
  check_null(max_in, "out of memory in repo_clique_get_cuts", CLEANUP);
  max_out = malloc(graph->nv * sizeof(struct vertex *));
  check_null(max_out, "out of memory in repo_clique_get_cuts", CLEANUP);

  for (i = 0; i < repo->size; i++)
  {
    clique = repo->cliques[i];

    graph->marker++;
    FOREACH_NODE_IN_CLIQUE(k, clique, j) { graph->v[k]->mark = graph->marker; }

    for (k = 0, ymax_in = 0.0, ymax_out = 0.0, fixed_in = 0, fixed_out = 0;
         k < graph->nv && fixed_in + fixed_out < 2; k++)
    {
      if (graph->v[k]->mark == graph->marker)
      {
        if (graph->v[k]->fixed)
        {
          nin           = 0;
          ymax_in       = graph->v[k]->y;
          max_in[nin++] = graph->v[k];
          fixed_in      = 1;
        }
        else if (!fixed_in)
        {
          if (ymax_in < graph->v[k]->y)
          {
            nin           = 0;
            ymax_in       = graph->v[k]->y;
            max_in[nin++] = graph->v[k];
          }
          else if (ymax_in < graph->v[k]->y + ZEROPLUS)
          {
            ymax_in       = graph->v[k]->y;
            max_in[nin++] = graph->v[k];
          }
        }
      }
      else
      {
        if (graph->v[k]->fixed)
        {
          nout            = 0;
          ymax_out        = graph->v[k]->y;
          max_out[nout++] = graph->v[k];
          fixed_out       = 1;
        }
        else if (!fixed_out)
        {
          if (ymax_out < graph->v[k]->y)
          {
            nout            = 0;
            ymax_out        = graph->v[k]->y;
            max_out[nout++] = graph->v[k];
          }
          else if (ymax_out < graph->v[k]->y + ZEROPLUS)
          {
            ymax_out        = graph->v[k]->y;
            max_out[nout++] = graph->v[k];
          }
        }
      }
    }

    nselin  = par->sec_max_vin < nin ? par->sec_max_vin : nin;
    nselout = par->sec_max_vout < nout ? par->sec_max_vout : nout;

    sel_in = malloc(nselin * sizeof(struct vertex *));
    check_null(sel_in, "out of memory in repo_clique_get_cuts", CLEANUP);
    sel_out = malloc(nselout * sizeof(struct vertex *));
    check_null(sel_out, "out of memory in repo_clique_get_cuts", CLEANUP);

    choose(sel_out, nselout, max_out, nout);

    // If a node d in the outside of the clique is fixed, i.e d->y=1, then this
    // can be simplified by max_out[0]=d, nout = 1;
    cut_val = 2 * ymax_in + 2 * ymax_out - 2 - clique->val;
    for (k = 0; k < nout && k < par->sec_max_vout; k++)
    {
      choose(sel_in, nselin, max_in, nin);
      for (j = 0; j < nin && j < par->sec_max_vin; j++)
      {
        if (cut_val > ZEROPLUS)
        {
          cut = cp_create_cut();
          check_null(cut, "out of memory in conv_subtour0cuts", CLEANUP);

          cut->hcount = 0;
          cut->tcount = 1;

          cut->teeth = malloc(sizeof(struct clique *));
          check_null(cut->teeth, "out of memory", CLEANUP);
          cut->teeth[0] = clique_create();
          check_null(cut->teeth[0], "out of memory", CLEANUP);
          clique_copy(clique, cut->teeth[0]);
          cut->verts    = malloc(2 * sizeof(int));
          cut->verts[0] = sel_in[j]->i;
          cut->verts[1] = sel_out[k]->i;
          cut->vycoef   = -2.0;

          cut->rhs   = -2.0;
          cut->sense = 'G';

          cut->next = *cuts;
          *cuts     = cut;
          (*cutcount)++;
        }
      }
    }
    free(sel_in);
    free(sel_out);
  }

CLEANUP:

  if (max_in)
    free(max_in);
  if (max_out)
    free(max_out);

  return rval;
}
