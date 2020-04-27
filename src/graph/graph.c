#include "../cpsrksec.h"

static void
graph_create_work(struct graph *graph),
graph_delete_work(struct graph *graph);

static void
graph_create_work(struct graph *graph)
{
  graph->nv_space  = 500;
  graph->na_space  = 100 * graph->nv_space;
  graph->nv        = 0;
  graph->n3v       = 0;
  graph->na        = 0;
  graph->v         = malloc(graph->nv_space * sizeof(struct vertex *));
  graph->marker    = 0;
  graph->connected = 0;
  graph->archash   = NULL;
  graph->arcs      = malloc(graph->na_space * sizeof(struct arc *));
  graph->shrunk    = NULL;
  graph->orig      = NULL;
  graph->tail      = NULL;
  graph->head      = NULL;
  graph_init_arc_hash(graph, 100);
  return;
}

struct graph *
graph_create(void)
{
  struct graph *graph = malloc(sizeof(struct graph));
  graph_create_work(graph);
  return graph;
}

int
graph_add_vertices(struct graph *graph, int nadd)
{
  int i, nv_old;
  assert(nadd >= 0);
  assert(graph->nv + nadd < NV_MAX);

  nv_old = graph->nv;
  if (graph->nv_space < graph->nv + nadd)
    realloc_scale(graph->v, graph->nv_space, graph->nv + nadd, 1.3);

  for (i = nv_old; i < nv_old + nadd; i++)
  {
    struct vertex *v;
    graph->v[i] = v = malloc(sizeof(struct vertex));
    v->i            = i;
    v->mark         = 0;
    v->mark_aux     = 0;
    v->fixed        = 0;
    v->branch       = 0;
    v->ind          = i;
    v->deg          = 0;
    v->coef         = 0.0;
    v->y            = 0.0;
    v->obj          = 0.0;
    v->vt_x         = 0.0;
    v->ut_x         = 0.0;
    v->comp         = 0;
    v->onqueue      = 0;
    v->active       = 0;
    v->edge         = NULL;

    v->orig     = NULL;
    v->shrunk   = NULL;
    v->members  = NULL;
    v->nmembers = 1;
    v->parent   = v;
    v->max      = v;

    v->prev = NULL;
    v->next = NULL;
    graph->nv++;
  }

  return nv_old;
}

struct arc *
graph_add_arc(struct graph *graph, int i, int j)
{
  struct arc *arc;

  assert(0 <= i && i < graph->nv);
  assert(0 <= j && j < graph->nv);

  if (i > j)
  {
    int temp;
    SWAP(i, j, temp);
  }

  arc = graph_find_arc_hash(graph->archash, i, j);
  if (!arc)
  {
    if (graph->na == NA_MAX)
    {
      printf("graph_add_arc: too many arcs\n");
      exit(1);
    }
    arc       = malloc(sizeof(struct arc));
    arc->tail = graph->v[i];
    arc->head = graph->v[j];

    arc->cost   = 0;
    arc->x      = 0.0;
    arc->obj    = 0.0;
    arc->branch = 0;
    arc->fixed  = 0;

    arc->flow = 0.0;

    arc->coef = 0;

    if (!graph->v[i]->deg)
    {
      if (i == 0)
      {
        if (graph->tail)
          graph->tail->prev = graph->v[i];
        else
          graph->head = graph->v[i];
        graph->v[i]->next = graph->tail;
        graph->tail       = graph->v[i];
      }
      else
      {
        if (graph->head)
          graph->head->next = graph->v[i];
        else
          graph->tail = graph->v[i];
        graph->v[i]->prev = graph->head;
        graph->head       = graph->v[i];
      }
      graph->n3v++;
    }
    if (!graph->v[j]->deg)
    {
      if (j == 0)
      {
        graph->v[j]->next = graph->tail;
        graph->tail->prev = graph->v[j];
        graph->tail       = graph->v[j];
      }
      else
      {
        graph->v[j]->prev = graph->head;
        graph->head->next = graph->v[j];
        graph->head       = graph->v[j];
      }
      graph->n3v++;
    }

    graph_add_arc_hash(graph->archash, arc);

    arc->t_prev = NULL;
    arc->t_next = graph->v[i]->edge;
    if (arc->t_next != NULL)
    {
      if (arc->t_next->tail->i == arc->tail->i)
        arc->t_next->t_prev = arc;
      else
        arc->t_next->h_prev = arc;
    }

    arc->h_prev = NULL;
    arc->h_next = graph->v[j]->edge;
    if (arc->h_next != NULL)
    {
      if (arc->h_next->head->i == arc->head->i)
        arc->h_next->h_prev = arc;
      else
        arc->h_next->t_prev = arc;
    }

    graph->v[i]->edge = graph->v[j]->edge = arc;

    if (graph->na + 1 >= graph->na_space)
    {
      realloc_scale(graph->arcs, graph->na_space, graph->na + 1, 1.3);
    }

    graph->arcs[graph->na] = arc;
    arc->ind               = graph->na;
    arc->i                 = graph->na;
    graph->na++;
    graph->v[i]->deg++;
    graph->v[j]->deg++;
  }

  return arc;
}

void
graph_del_arc(struct graph *graph, struct arc **arc)
{
  int rval = 0;
  int i;
  struct vertex *tail, *head;
  assert(graph->na > 0);
  assert(0 <= (*arc)->tail->i && (*arc)->tail->i < graph->nv);
  assert((*arc)->tail == graph->v[(*arc)->tail->i]);
  assert(0 <= (*arc)->head->i && (*arc)->head->i < graph->nv);
  assert((*arc)->head == graph->v[(*arc)->head->i]);
  tail = graph->v[(*arc)->tail->i];
  head = graph->v[(*arc)->head->i];

  if (graph_find_arc_hash(graph->archash, (*arc)->tail->i, (*arc)->head->i))
  {

    if ((*arc)->h_prev == NULL)
      (*arc)->head->edge = (*arc)->h_next;
    else
    {
      if ((*arc)->h_prev->head->i == (*arc)->head->i)
        (*arc)->h_prev->h_next = (*arc)->h_next;
      else
        (*arc)->h_prev->t_next = (*arc)->h_next;
    }
    if ((*arc)->h_next == NULL)
      ;
    else
    {
      if ((*arc)->h_next->head->i == (*arc)->head->i)
      {
        if ((*arc)->h_prev)
          (*arc)->h_next->h_prev = (*arc)->h_prev;
        else
          (*arc)->h_next->h_prev = NULL;
      }
      else
        (*arc)->h_next->t_prev = (*arc)->h_prev;
    }

    if ((*arc)->t_prev)
    {
      if ((*arc)->t_prev->tail->i == (*arc)->tail->i)
        (*arc)->t_prev->t_next = (*arc)->t_next;
      else
        (*arc)->t_prev->h_next = (*arc)->t_next;
    }
    else
      (*arc)->tail->edge = (*arc)->t_next;
    if ((*arc)->t_next)
    {
      if ((*arc)->t_next->tail->i == (*arc)->tail->i)
        (*arc)->t_next->t_prev = (*arc)->t_prev;
      else
        (*arc)->t_next->h_prev = (*arc)->t_prev;
    }
    else
    {
      if ((*arc)->t_prev != NULL)
      {
        if ((*arc)->t_prev->tail->i == (*arc)->tail->i)
          (*arc)->t_prev->t_next = NULL;
        else
          (*arc)->t_prev->h_next = NULL;
      }
    }

    rval = graph_del_arc_hash(graph->archash, (*arc));
    check_rval(rval, "arc hash not found\n", CLEANUP);

    graph->na--;

    if ((*arc)->i < graph->na && graph->na > 0)
    {

      i                      = (*arc)->i;
      graph->arcs[i]         = graph->arcs[graph->na];
      graph->arcs[graph->na] = NULL;
      graph->arcs[i]->i      = i;
    }

    graph->arcs[graph->na] = NULL;

    tail->deg--;
    head->deg--;
    if (!tail->deg)
    {
      if (tail->next)
        tail->next->prev = tail->prev;
      else
        graph->head = tail->prev;
      if (tail->prev)
        tail->prev->next = tail->next;
      else
        graph->tail = tail->next;
      graph->n3v--;
      tail->prev = NULL;
      tail->next = NULL;
    }
    if (!head->deg)
    {
      if (head->next)
        head->next->prev = head->prev;
      else
        graph->head = head->prev;
      if (head->prev)
        head->prev->next = head->next;
      else
        graph->tail = head->next;
      graph->n3v--;
      head->prev = NULL;
      head->next = NULL;
    }
    (*arc)->tail->y -= (*arc)->x / 2.0;
    (*arc)->head->y -= (*arc)->x / 2.0;

    free((*arc));
  }
  else
  {
    printf("not found %d %d\n", (*arc)->tail->i, (*arc)->head->i);
    exit(1);
  }

CLEANUP:
  return;
}

static void
graph_delete_work(struct graph *graph)
{
  int i;
  if (graph->archash)
    graph_free_arc_hash(&(graph->archash));
  for (i = 0; i < graph->nv; i++) free(graph->v[i]);
  free(graph->v);
  for (i = 0; i < graph->na; i++) free(graph->arcs[i]);
  free(graph->arcs);
  if (graph->shrunk)
  {
    graph_free(graph->shrunk);
  }
  return;
}

void
graph_erase(struct graph *graph)
{
  graph_delete_work(graph);
  graph_create_work(graph);
  return;
}

void
graph_free(struct graph *graph)
{
  graph_delete_work(graph);
  free(graph);
  return;
}

struct arc *
graph_find_arc(struct graph *graph, struct vertex *tail, struct vertex *head)
{
  int end0, end1, t;
  end0 = tail->i;
  end1 = head->i;

  if (end0 > end1)
    SWAP(end0, end1, t);

  return graph_find_arc_hash(graph->archash, end0, end1);
}

int
graph_copy(struct graph *ingraph, struct graph *outgraph)
{
  int rval = 0;
  int i;
  struct vertex *v1, *v2;
  struct arc *e;
  struct arc *arc;

  outgraph->orig  = ingraph;
  ingraph->shrunk = outgraph;

  graph_add_vertices(outgraph, ingraph->nv);

  for (i = 0; i < ingraph->nv; i++)
  {
    outgraph->v[i]->fixed = ingraph->v[i]->fixed;
    outgraph->v[i]->obj   = ingraph->v[i]->obj;
    outgraph->v[i]->orig  = ingraph->v[i];
    ingraph->v[i]->shrunk = outgraph->v[i];
  }

  (outgraph->marker)++;
  outgraph->n3v = 0;
  for (i = 0; i < ingraph->na; i++)
  {
    arc = ingraph->arcs[i];
    {
      e  = graph_add_arc(outgraph, arc->tail->i, arc->head->i);
      v1 = outgraph->v[arc->tail->i];
      v2 = outgraph->v[arc->head->i];

      e->x    = arc->x;
      e->cost = arc->cost;
      e->obj  = arc->obj;
      e->tail->y += e->x / 2.0;
      e->head->y += e->x / 2.0;
    }
  }
  return rval;
}

void
graph_print(struct graph *graph)
{
  int i;
  struct vertex *v, *other;
  struct arc *e, *enext;

  printf("\n### Support Graph\n");

  for (i = 0; i < graph->nv; i++)
  {
    v = graph->v[i];
    if (v->y)
    {
      printf("    Node %d [%.3f]:", v->i, v->y);
      for (e = v->edge; e; e = enext)
      {
        enext = outnext(e, v);
        other = otherend(e, v);
        printf("%d [%.3f] ", other->i, e->x);
      }
      printf("\n");
    }
  }

  printf("### End Graph\n\n");
}

void
graph_write(struct graph *graph)
{
  int i;
  FILE *file;
  struct vertex *v, *other;
  struct arc *e, *enext;

  printf("\n");
  printf("Writing graph to '%s'...\n", "graph-saved.txt");

  file = fopen("graph-saved-gsec.txt", "w");
  if (file == NULL)
  {
    printf("Unable to create '%s'\n", "saved.graph");
    exit(1);
  }

  fprintf(file, "Number_of_nodes: %d\n", graph->nv);
  fprintf(file, "Number_of_arcs: %d\n", graph->na);
  for (i = 0; i < graph->nv; i++)
  {
    v = graph->v[i];
    if (v->y)
    {
      fprintf(file, "Node %d [%.6f]:", v->i, v->y);
      for (e = v->edge; e; e = enext)
      {
        enext = outnext(e, v);
        other = otherend(e, v);
        if (e->tail->i == v->i)
          fprintf(file, " %d [%.6f]", other->i,
                  roundf(e->x * ((double)10000000)) / ((double)10000000));
      }
      fprintf(file, "\n");
    }
  }

  if (file != NULL)
    fclose(file);
}

struct graph *
graph_read(char *fname)
{
  FILE *file;
  int nv, na;
  struct arc *arc;
  char buf[256];
  struct graph *graph = graph_create();

  file = fopen(fname, "r");
  if (file == NULL)
  {
    printf("Unable to open '%s'\n", fname);
    goto done;
  }

  fscanf(file, "%*s %d", &nv);
  graph_add_vertices(graph, nv);

  fscanf(file, "%*s %d", &na);

  int tail, head, n_read = 0;
  double node_val, arc_val;
  char *read_ptr = buf;
  while (fgets(buf, 254, file) != NULL)
  {
    if (sscanf(buf, "%*s %d [%lf]: %n", &tail, &node_val, &n_read) > 0)
    { // printf("NODE %d VAL %f\n", tail, node_val);
      graph->v[tail]->y = node_val;
      read_ptr += n_read;
    }
    while (sscanf(read_ptr, "%d [%lf] %n", &head, &arc_val, &n_read) == 2)
    { // printf("  arc %d %f\n", head, arc_val);
      arc    = graph_add_arc(graph, tail, head);
      arc->x = arc_val;
      read_ptr += n_read;
    }
    read_ptr = buf;
  }

  if (file != NULL)
    fclose(file);

  return graph;

done:
  return NULL;
}

void
graph_identify_vertices(struct graph *graph, struct vertex *v, struct vertex *u)
{
  double oldobj, oldcost;
  struct arc *e, *new, *old, *next;
  struct vertex *other;

  assert(v->i != u->i);

  u->parent = v;

  if (!v->members)
  {
    v->members = u;
  }
  else if (!u->members)
  {
    u->members = v->members;
    v->members = u;
  }
  else
  {
    struct vertex *t;
    for (t = v->members; t->members; t = t->members)
      ;
    t->members = u;
  }
  v->nmembers += u->nmembers;
  v->obj += u->obj;

  if (v->max->orig->y < u->max->orig->y)
    v->max = u->max;

  old     = graph_find_arc_hash(graph->archash, v->i, u->i);
  oldobj  = old ? old->obj : 0;
  oldcost = old ? old->cost : 0;
  for (e = u->edge; e; e = next)
  {
    other = otherend(e, u);
    next  = outnext(e, u);
    if (other->i != v->i)
    {
      new = graph_add_arc(graph, v->i, other->i);
      new->flow += e->flow;

      new->x += e->x;
      new->obj  = e->obj + u->obj + oldobj;
      new->cost = e->cost + oldcost;
      v->y += e->x / 2.0;
      other->y += e->x / 2.0;
    }

    graph_del_arc(graph, &e);
  }
}

int
graph_expand_vertex(struct graph *graph, struct vertex *srkv, int *vcount,
                    struct vertex ***verts)
{
  int rval = 0;
  int cnt;
  struct vertex **tverts;
  struct vertex *v;

  *vcount = 0;
  *verts  = NULL;

  cnt = 1;
  for (v = srkv->members; v; v = v->members) cnt++;

  tverts = malloc(cnt * sizeof(struct vertex *));
  check_null(tverts, "graph_expand_vertex failed", CLEANUP);

  tverts[0] = srkv->orig;
  cnt       = 1;
  for (v = srkv->members; v; v = v->members) tverts[cnt++] = v->orig;

  *vcount = cnt;
  *verts  = tverts;

CLEANUP:

  return rval;
}

static int
sort_nodes(const void *vv, const void *uu);
int
graph_reorder_vertices(struct graph *graph)
{
  int i, k;

  struct vertex *v;

  struct vertex **nnvertices = NULL;

  // Sort nodes
  nnvertices = malloc(graph->n3v * sizeof(struct vertex *));
  for (v = graph->tail, k = 0; v; v = v->next) nnvertices[k++] = v;
  assert(k == graph->n3v);
  qsort(nnvertices, k, sizeof(struct vertex *), sort_nodes);

  graph->tail = nnvertices[0];
  v           = nnvertices[0];
  v->prev     = NULL;
  v->mark_aux = 1;
  for (i = 1; i < k; i++)
  {
    v->next             = nnvertices[i];
    nnvertices[i]->prev = v;
    v                   = nnvertices[i];
    v->next             = NULL;
    v->mark_aux         = 1;
  }
  graph->head = v;

  if (nnvertices)
    free(nnvertices);

  return 0;
}

static int
sort_nodes(const void *vv, const void *uu)
{
  struct vertex *v = *(struct vertex **)vv, *u = *(struct vertex **)uu;

#if REORDER == 0
  if (v->fixed && u->fixed)
  {
    if (rand() % 2)
      return -1;
    else
      return +1;
  }
#elif REORDER == 1
  if (v->fixed && u->fixed)
  {
    if (v->i < u->i)
      return -1;
    if (v->i > u->i)
      return +1;
  }
#else
  if (v->fixed && u->fixed)
    return 0;
#endif
  if (v->fixed)
    return -1;
  if (u->fixed)
    return +1;

  if (v->y + ZEROPLUS < u->y)
    return +1;
  if (v->y - ZEROPLUS > u->y)
    return -1;

#if REORDER == 0
  if (rand() % 2)
    return -1;
  else
    return +1;
#elif REORDER == 1
  if (v->i < u->i)
    return -1;
  if (v->i > u->i)
    return +1;
#else
  return 0;
#endif
}
