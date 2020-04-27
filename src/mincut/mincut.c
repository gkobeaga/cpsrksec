#include "../cpsrksec.h"

static int
get_vertices(struct graph *graph, struct vertex *v, struct vertex ***verts,
             int *vcount);

int
mincut_solve(struct graph *graph, struct vertex *s, struct vertex *t,
             double *value, struct vertex ***verts, int *vcount)
{
  int rval = 0;

  if (verts)
  {
    *verts = NULL;
    if (vcount)
      *vcount = 0;
    else
    {
      if (rval)
      {
        printf("verts is specified but not vcount\n");
        goto CLEANUP;
      }
    }
  }

  *value = maxflow_solve(graph, s, t);

  if (verts)
  {
    rval = get_vertices(graph, t, verts, vcount);
    if (rval)
    {
      printf("get_vertices failed\n");
      goto CLEANUP;
    }
  }

  return 0;

CLEANUP:
  return 1;
}

static int
get_vertices(struct graph *graph, struct vertex *t, struct vertex ***verts,
             int *vcount)
{
  int rval = 0;
  struct arc *e;
  struct vertex *q, *top, *other;
  int count = 0;
  int i, marker;
  int *vid = NULL;

  *verts  = NULL;
  *vcount = 0;

  vid = malloc(graph->nv * sizeof(int));
  if (!vid)
  {
    printf("out of memory in get_vertices\n");
    goto CLEANUP;
  }

  marker         = ++(graph->marker);
  vid[count++]   = t->i;
  q              = t;
  q->mark        = marker;
  q->search_next = NULL;

  while (q)
  {
    top = q;
    q   = q->search_next;
    for (e = top->edge; e; e = outnext(e, top))
    {
      other = otherend(e, top);
      if (e->tail->i == top->i)
      {
        if (e->x + e->flow > 0.0 && other->mark != marker)
        {
          vid[count++]       = other->i;
          other->mark        = marker;
          other->search_next = q;
          q                  = other;
        }
      }
      else
      {
        other = otherend(e, t);
        if (e->x - e->flow > 0.0 && other->mark != marker)
        {
          vid[count++]       = other->i;
          other->mark        = marker;
          other->search_next = q;
          q                  = other;
        }
      }
    }
  }

  *verts = malloc(count * sizeof(struct vertex *));
  if (!(*verts))
  {
    printf("out of memory in get_vertices\n");
    goto CLEANUP;
  }

  for (i = 0; i < count; i++) (*verts)[i] = graph->v[vid[i]];

  *vcount = count;

  if (vid)
    free(vid);
  return rval;

CLEANUP:

  rval = 1;

  if (*verts)
    free(*verts);
  return rval;
}
