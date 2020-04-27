#include "../../cpsrksec.h"

int
cp_shrink_s1c3(struct cp_par *par, struct graph *graph, struct vertex *qstart, struct repo *repo)
{
  int rval = 0;
  struct vertex *v, *u, *w, *t;
  struct arc *e, *e_vu, *e_ut, *e_uw, *e_wt;
  struct vertex *qhead, *qtail, *other;
  int try_c3;
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

    u    = NULL;
    e_vu = NULL;
    for (e_vu = v->edge;
         e_vu && (fabs(e_vu->x - c) >= ZEROPLUS ||
                  fabs(e_vu->tail->y - e_vu->head->y) >= ZEROPLUS);
         e_vu = outnext(e_vu, v))
    { // Rule S2
      if (par->srk_s2 && graph->n3v > 2)
      {
        u = otherend(e_vu, v);
        if ( e_vu->x > v->y + C2_ZEROPLUS && e_vu->x > u->y + C2_ZEROPLUS &&
             v->i != graph->tail->i && u->i != graph->tail->i )
        {
          graph_identify_vertices(graph, v, u);
          par->count_s2++;
            ADD_TO_SRK_QUEUE(v);
          for (e = v->edge; e; e = outnext(e, v))
          {
            other = otherend(e, v);
            if (e->x > other->y + ZEROPLUS)
            {
              rval = repo_cp_save_vertices(graph, v, other, e->x, repo);
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

    if (e_vu)
      u = otherend(e_vu, v);

    if (e_vu && u->i != graph->tail->i)
    { // Rule S1

      graph_identify_vertices(graph, v, u);
      par->count_s1++;

      ADD_TO_SRK_QUEUE(v);
      for (e = v->edge; e; e = outnext(e, v))
      {
        other = otherend(e, v);
        if (e->x > other->y + ZEROPLUS)
        {
          rval = repo_cp_save_vertices(graph, v, other, e->x, repo);
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
    /* To avoid reordering the nodes */
    else if (e_vu && u->i == graph->tail->i)
    { ADD_TO_SRK_QUEUE(u);
      goto GET_OUT;
    }

    /* Rule C3 */
    try_c3 = 0;
    for (e_vu = v->edge; e_vu; e_vu = outnext(e_vu, v))
    {
      u       = otherend(e_vu, v);
      u->vt_x = e_vu->x;
      if (!try_c3 && fabs(u->y - c) < ZEROPLUS)
        try_c3++;
    }

    if (try_c3)
    {
      for (e_vu = v->edge; e_vu; e_vu = outnext(e_vu, v))
      {
        u = otherend(e_vu, v);
        if (fabs(u->y - c) < ZEROPLUS)
        {
          for (e_ut = u->edge; e_ut; e_ut = outnext(e_ut, u))
          {
            t       = otherend(e_ut, u);
            t->ut_x = e_ut->x;
          }
          for (e_uw = u->edge; e_uw; e_uw = outnext(e_uw, u))
          {
            w = otherend(e_uw, u);
            if (w->i != v->i && fabs(w->y - c) < ZEROPLUS)
            {
              if (fabs(e_vu->x + e_uw->x + w->vt_x - 2 * c) < ZEROPLUS)
              {
                for (e_wt = w->edge; e_wt; e_wt = outnext(e_wt, w))
                {
                  t = otherend(e_wt, w);
                  if (t->i != v->i && t->i != u->i && fabs(t->y - c) < ZEROPLUS)
                  {
                    if (fabs(e_wt->x + t->vt_x + t->ut_x - c) < ZEROPLUS)
                    {
                      for (e_ut = u->edge; e_ut; e_ut = outnext(e_ut, u))
                      {
                        t       = otherend(e_ut, u);
                        t->ut_x = 0.0;
                      }
                      graph_identify_vertices(graph, v, u);
                      graph_identify_vertices(graph, v, w);
                      par->count_c3++;

                      ADD_TO_SRK_QUEUE(v);
                      for (e = v->edge; e; e = outnext(e, v))
                      {
                        other = otherend(e, v);

                        if (e->x > other->y + ZEROPLUS)
                        {
                          rval =
                          repo_cp_save_vertices(graph, v, other, e->x, repo);
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
          for (e_ut = u->edge; e_ut; e_ut = outnext(e_ut, u))
          {
            t       = otherend(e_ut, u);
            t->ut_x = 0.0;
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
