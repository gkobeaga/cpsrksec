#include "../../cpsrksec.h"

int
cp_shrink_c1c2(struct cp_par *par, struct graph *graph, struct vertex *qstart,
               struct repo *repo)
{
  int rval = 0;
  struct vertex *v, *u, *t;
  struct arc *e, *e_vu, *e_ut, *h, *next;
  struct vertex *qhead, *qtail, *other;
  int try_c1c2;
  double c;

  if (qstart)
  {
    qhead = qstart;
    for (v = qstart; v->qnext; v = v->qnext) v->onqueue = 1;
    qtail          = v;
    qtail->onqueue = 1;
  }
  else
  {
    for (v = graph->tail; v->next; v = v->next)
    {
      v->qnext   = v->next;
      v->onqueue = 1;
    }
    qhead          = graph->tail;
    qtail          = v;
    qtail->onqueue = 1;
    qtail->qnext   = NULL;
  }

  while (qhead)
  {
    v     = qhead;
    qhead = qhead->qnext;
    if (!qhead)
      qtail = NULL;
    if (v->parent != v)
      continue;
    v->onqueue = 0;

    par->count_queue++;

    c = v->y;

    // Cycle Rules
    try_c1c2 = 0;
    for (e_vu = v->edge; e_vu; e_vu = outnext(e_vu, v))
    {
      u       = otherend(e_vu, v);
      u->vt_x = e_vu->x;
      if (!try_c1c2 && fabs(u->y - c) < ZEROPLUS && fabs(e_vu->x - c) < ZEROPLUS)
        try_c1c2++;
    }

    if (try_c1c2)
    {
      for (e_vu = v->edge; e_vu; e_vu = outnext(e_vu, v))
      {
        u = otherend(e_vu, v);
        if (fabs(u->y - c) < ZEROPLUS && fabs(e_vu->x - c) < ZEROPLUS)
        { // Rule 1 and 2
          for (e_ut = u->edge; e_ut; e_ut = next)
          {
            next = outnext(e_ut, u);
            t    = otherend(e_ut, u);
            if (t->i != v->i && fabs(t->y - c) < ZEROPLUS)
            { // x({u,v}:t)=c
              if (fabs(e_ut->x + t->vt_x - c) < ZEROPLUS)
              {
                graph_identify_vertices(graph, v, u);
                if (t->vt_x < ZEROPLUS)
                  par->count_c1++;
                else
                  par->count_c2++;

                ADD_TO_SRK_QUEUE(v);
                for (h = v->edge; h; h = outnext(h, v))
                {
                  other = otherend(h, v);

                  if (h->x > other->y + ZEROPLUS)
                  {
                    rval = repo_cp_save_vertices(graph, v, other, h->x, repo);
                    if (rval)
                    {
                      printf("repo_cp_save_vertices failed\n");
                      goto CLEANUP;
                    }
                  }
                  ADD_TO_SRK_QUEUE(other);
                }
                goto GET_OUT;
              }
            }
          }
        }
      }
    }
  GET_OUT:
    for (e = v->edge; e; e = outnext(e, v))
    {
      other       = otherend(e, v);
      other->vt_x = 0.0;
    }
  }

CLEANUP:

  return rval;
}
