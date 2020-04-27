#include "../../cpsrksec.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct clique *
clique_create(void)
{
  struct clique *clique = malloc(sizeof(struct clique));
  clique->segcount      = 0;
  clique->nodes         = NULL;
  clique->hashnext      = 0;
  clique->refcount      = 0;
  clique->val           = 0.0;
  clique->prev          = NULL;
  clique->next          = NULL;
  return clique;
}

static int
sort_vert(const void *xx, const void *yy)
{
  struct vertex *x = *(struct vertex **)xx, *y = *(struct vertex **)yy;

  if (x->i < y->i)
    return -1;
  if (x->i > y->i)
    return +1;

  return 0;
}

struct clique *
conv_vertices2clique(struct graph *graph, struct vertex **vert, int acount)
{
  struct clique *clique = clique_create();
  int i, nseg;

  qsort(vert, acount, sizeof(struct vertex *), sort_vert);

  nseg = 0;

  i = 0;
  while (i < acount)
  {
    assert(i == (acount - 1) || vert[i + 1]->i != vert[i]->i);
    while (i < (acount - 1) && vert[i + 1]->i == (vert[i]->i + 1)) i++;
    i++;
    nseg++;
  }

  clique->nodes = malloc(nseg * sizeof(struct segment));
  if (!clique->nodes)
  {
    fprintf(stderr, "out of memory in conv_vertices2clique\n");
    free(clique);
    return NULL;
  }
  clique->segcount = nseg;

  nseg = 0;
  i    = 0;
  while (i < acount)
  {
    clique->nodes[nseg].lo = vert[i]->i;
    while (i < (acount - 1) && vert[i + 1]->i == (vert[i]->i + 1)) i++;
    clique->nodes[nseg].hi = vert[i++]->i;
    nseg++;
  }

  return clique;
}

struct clique *
conv_vertices2coclique(struct graph *graph, struct vertex **vert, int acount)
{
  struct clique *clique = clique_create();
  int i, nseg;

  qsort(vert, acount, sizeof(struct vertex *), sort_vert);

  nseg = 0;

  i = 0;
  while (i < acount)
  {
    assert(i == (acount - 1) || vert[i + 1]->i != vert[i]->i);
    if (0 < vert[i]->i)
      nseg++;
    while (i < (acount - 1) && vert[i + 1]->i == (vert[i]->i + 1)) i++;
    i++;
  }
  if (vert[acount - 1]->i < graph->nv - 1)
    nseg++;

  clique->nodes = malloc(nseg * sizeof(struct segment));
  if (!clique->nodes)
  {
    fprintf(stderr, "out of memory in conv_vertices2coclique\n");
    free(clique);
    return NULL;
  }
  clique->segcount = nseg;

  nseg = 0;
  i    = 0;
  if (vert[i]->i)
  {
    clique->nodes[nseg].lo = 0;
    clique->nodes[nseg].hi = vert[i]->i - 1;
    nseg++;
  }
  while (i < (acount - 1) && vert[i + 1]->i == (vert[i]->i + 1)) i++;
  while (i < acount - 1)
  {
    clique->nodes[nseg].lo = vert[i]->i + 1;
    clique->nodes[nseg].hi = vert[++i]->i - 1;
    nseg++;
    while (i < (acount - 1) && vert[i + 1]->i == (vert[i]->i + 1)) i++;
  }
  if (vert[i]->i < graph->nv - 1)
  {
    clique->nodes[nseg].lo = vert[i]->i + 1;
    clique->nodes[nseg].hi = graph->nv - 1;
    nseg++;
  }

  return clique;
}

int
clique_copy(struct clique *in, struct clique *out)
{
  int rval = 0;
  int k;
  struct segment *s = NULL;

  if (in->segcount)
  {
    s = malloc(in->segcount * sizeof(struct segment));
    if (!s)
    {
      printf("out of memory in clique_copy");
      goto CLEANUP;
    }
    for (k = 0; k < in->segcount; k++)
    {
      s[k].lo = in->nodes[k].lo;
      s[k].hi = in->nodes[k].hi;
    }
  }
  out->segcount = in->segcount;
  out->val      = in->val;
  out->nodes    = s;
CLEANUP:

  return rval;
}

void
clique_free(struct clique *clique)
{
  if (clique->nodes)
  {
    free(clique->nodes);
  }
  free(clique);
}

unsigned int clique_hash(struct clique* c)
{
  unsigned int x = 0;
  int i;

  for (i = 0; i < c->segcount; i++) {
    struct segment seg = c->nodes[i];
    x = x * 65537 + seg.lo * 4099 + seg.hi;
  }

  return x;
}

int clique_eq(struct clique* c, struct clique* d)
{
  int i;

  if (c->segcount != d->segcount)
    return 0;
  for (i = 0; i < c->segcount; i++) {
    struct segment seg1 = c->nodes[i];
    struct segment seg2 = d->nodes[i];
    if (seg1.lo != seg2.lo)
      return 0;
    if (seg1.hi != seg2.hi)
      return 0;
  }
  return 1;
}

int
clique_count(struct clique *clique)
{
  int i;
  int count             = 0;
  struct segment *nodes = clique->nodes;

  for (i = 0; i < clique->segcount; i++)
    count += (nodes[i].hi - nodes[i].lo + 1);
  return count;
}

void
clique_print(struct clique *clique)
{
  int i, tmp;
  int count1, count2 = 0;

  printf("\nClique: (count %d)\n", count1 = clique_count(clique));
  FOREACH_NODE_IN_CLIQUE(i, clique, tmp)
  {
    count2++;
    printf(" %d", i);
  }
  printf("\n");
  assert(count1 == count2);

  return;
}
