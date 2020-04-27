#include "../cpsrksec.h"

static void
create_cut(struct cpcut *cut)
{
  cut->hcount  = 0;
  cut->tcount  = 0;
  cut->rhs     = 0.0;
  cut->sense   = 'X';
  cut->handles = NULL;
  cut->teeth   = NULL;
  cut->cliques = NULL;
  cut->verts   = NULL;
  cut->vycoef  = 0.0;
  cut->next    = NULL;
  cut->prev    = NULL;
}

struct cpcut *
cp_create_cut(void)
{
  struct cpcut *cut = malloc(sizeof(struct cpcut));
  create_cut(cut);
  return cut;
}

void
cp_free_cut(struct cpcut **cut)
{
  int i;

  if (*cut)
  {
    if ((*cut)->hcount + (*cut)->tcount)
    {
      if ((*cut)->handles)
      {
        for (i = 0; i < (*cut)->hcount; i++)
        {
          if ((*cut)->handles[i])
            clique_free((*cut)->handles[i]);
        }
        free((*cut)->handles);
      }
      if ((*cut)->teeth)
      {
        for (i = 0; i < (*cut)->tcount; i++)
        {
          if ((*cut)->teeth[i])
            clique_free((*cut)->teeth[i]);
        }
        free((*cut)->teeth);
      }
      if ((*cut)->verts)
        free((*cut)->verts);
    }
  }
  free(*cut);
  *cut = NULL;
}

double
cp_eval_cut(struct graph *graph, struct cpcut *cut)
{
  int i;
  double slack = 0.0;
  struct arc *arc;

  if (cut->tcount + cut->hcount)
  {
    slack -= cut->rhs;

    int nzcnt          = 0;
    struct arc **nzlist = malloc(graph->na * sizeof(struct arc *));
    cp_get_cut_arcs(graph, cut, &nzcnt, nzlist);

    for (i = 0; i < nzcnt; i++)
    {
      arc = nzlist[i];
      if (arc->coef)
      {
        slack += arc->coef * arc->x;
        arc->coef = 0;
      }
    }

    for (i = 0; i < 2 * cut->tcount; i++)
    {
      if (cut->verts[i] >= 0)
        slack += cut->vycoef * graph->v[cut->verts[i]]->y;
    }

    if (cut->sense == 'L')
      slack = -slack;

    free(nzlist);
  }

  return slack;
}

void
cp_get_cut_arcs(struct graph *graph, struct cpcut *cut, int *nzcnt, struct arc **nzlist)
{
  int marker;
  int i, tmp, k;
  struct arc *arc;
  struct vertex *v, *other;
  struct clique *tooth, *handle;

  *nzcnt = 0;

  if (cut->tcount + cut->hcount)
  {
    for (i = 0; i < cut->hcount; i++)
    {
      handle = cut->handles[i];
      (graph->marker)++;
      marker = graph->marker;

      FOREACH_NODE_IN_CLIQUE(k, handle, tmp) { graph->v[k]->mark = marker; }

      FOREACH_NODE_IN_CLIQUE(k, handle, tmp)
      {
        v = graph->v[k];
        for (arc = v->edge; arc; arc = outnext(arc, v))
        {
          other = otherend(arc, v);
          if (other->mark != marker)
          {
            if (!arc->coef)
              nzlist[(*nzcnt)++] = arc;
            arc->coef += 1;
          }
        }
      }
    }

    for (i = 0; i < cut->tcount; i++)
    {
      tooth = cut->teeth[i];
      (graph->marker)++;
      marker = graph->marker;
      FOREACH_NODE_IN_CLIQUE(k, tooth, tmp) { graph->v[k]->mark = marker; }

      FOREACH_NODE_IN_CLIQUE(k, tooth, tmp)
      {
        v = graph->v[k];
        for (arc = v->edge; arc; arc = outnext(arc, v))
        {
          other = otherend(arc, v);
          if (other->mark != marker)
          {
            if (!arc->coef)
              nzlist[(*nzcnt)++] = arc;
            arc->coef += 1;
          }
        }
      }
    }
  }
}

void
cp_print_cut(struct cpcut *cut)
{
  int i,j,k;
  if (cut->hcount + cut->tcount)
    printf("CP Cut:\n");
  if (cut->tcount)
  {
    printf(" - Verts:");
    for (i = 0; i < cut->tcount; i++)
    {
      printf("  %d, %d\n", cut->verts[2 * i], cut->verts[2 * i + 1]);
    }
  }
  if (cut->hcount)
  {
    printf(" - Handle: ");
    if (cut->handles)
    {
      FOREACH_NODE_IN_CLIQUE(j, cut->handles[0], k)
        printf(" %d", j);
      printf("\n");
    }
  }
  if (cut->tcount)
  {
    printf(" - Teeth [%d]\n", cut->tcount);
    int count;
    for (i = 0; i < cut->tcount; i++)
    {
      printf("   + Tooth-%d: ", i);
      if (cut->teeth)
        count = clique_count(cut->teeth[i]);
      assert(count != 0);
      if (cut->teeth)
      {
        FOREACH_NODE_IN_CLIQUE(j, cut->teeth[i], k)
          printf(" %d", j);
        printf("\n");
      }
    }
  }
}
