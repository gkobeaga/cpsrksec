#include "../../cpsrksec.h"

int
cp_shrink_s1(struct cp_par *par, struct graph *graph, struct vertex *qstart, struct repo *repo)
{
  int rval = 0;
  struct vertex *v, *u;
  struct arc *e_vu = NULL, *h;
  struct vertex *qhead, *qtail, *other;
  double c;
  int reorder;

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
    {
      if (par->srk_s2 && graph->n3v > 2)
      {
        u = otherend(e_vu, v);
        if ( e_vu->x > v->y + C2_ZEROPLUS && e_vu->x > u->y + C2_ZEROPLUS )
        {
          if ( v->i != graph->tail->i && u->i != graph->tail->i )
            reorder = 0;
          else
            reorder = 1;

          graph_identify_vertices(graph, v, u);
          par->count_s2++;
          ADD_TO_SRK_QUEUE(v);
          for (h = v->edge; h; h = outnext(h, v))
          {
            other = otherend(h, v);
            if ( h->x > other->y + ZEROPLUS )
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

          if (reorder)
            graph_reorder_vertices(graph);

          goto GET_OUT;
        }
      }
    }

    if (e_vu)
      u = otherend(e_vu, v);

    /* Rule S1 */
    if (e_vu && u->i != graph->tail->i)
    {
      graph_identify_vertices(graph, v, u);
      par->count_s1++;

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
    }
    else if (e_vu && u->i == graph->tail->i)
    { // To avoid reordering the nodes.
      ADD_TO_SRK_QUEUE(u);
    }
  GET_OUT:;
  }

CLEANUP:

  return rval;
}
