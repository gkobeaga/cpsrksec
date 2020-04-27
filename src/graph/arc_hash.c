#include "../cpsrksec.h"

int
graph_init_arc_hash(struct graph *graph, int size)
{
  unsigned int i;
  graph->archash = malloc(sizeof(struct arc_hash));

  graph->archash->size  = prime_next((unsigned int)size);
  graph->archash->mult  = (int)sqrt((double)graph->archash->size);
  graph->archash->table = malloc(graph->archash->size * sizeof(struct arc *));

  if (!graph->archash->table)
  {
    graph->archash->size = 0;
    return 1;
  }
  for (i = 0; i < graph->archash->size; i++) graph->archash->table[i] = NULL;
  return 0;
}

void
graph_add_arc_hash(struct arc_hash *hash, struct arc *arc)
{
  unsigned int loc;

  if (hash->size == 0)
    return;

  int end0, end1;
  end0 = arc->tail->i;
  end1 = arc->head->i;

  loc = (end0 * hash->mult + end1) % hash->size;

  arc->hash_prev = NULL;
  arc->hash_next = hash->table[loc];

  if (arc->hash_next)
    arc->hash_next->hash_prev = arc;

  hash->table[loc] = arc;
  return;
}

int
graph_del_arc_hash(struct arc_hash *hash, struct arc *arc)
{
  int end0, end1;
  int hashval;
  struct arc *parc;

  end0 = arc->tail->i;
  end1 = arc->head->i;

  if (hash->size == 0)
    return 1;

  hashval = (end0 * hash->mult + end1) % hash->size;
  for (parc = hash->table[hashval]; parc; parc = parc->hash_next)
  {
    if (parc->tail->i == end0 && parc->head->i == end1)
    {
      if (parc->hash_prev == NULL && parc->hash_next == NULL)
      {
        hash->table[hashval] = NULL;
      }
      else
      {
        if (parc->hash_prev)
          parc->hash_prev->hash_next = parc->hash_next;
        else
          hash->table[hashval] = parc->hash_next;

        if (parc->hash_next)
          parc->hash_next->hash_prev = parc->hash_prev;
      }
      return 0;
    }
  }
  return 1;
}

int
graph_getall_arc_hash(struct arc_hash *hash, int *narcs, struct arc ***arcs)
{
  unsigned int i;
  int k = 0;
  struct arc *arc;

  for (i = 0; i < hash->size; i++)
  {
    for (arc = hash->table[i]; arc; arc = arc->hash_next) k++;
  }

  if (k > 0)
  {
    *narcs = k;
    *arcs  = malloc(*narcs * sizeof(struct arc *));
    for (i = 0, k = 0; i < hash->size; i++)
    {
      for (arc = hash->table[i]; arc != NULL; arc = arc->hash_next)
      {
        (*arcs)[k++] = arc;
      }
    }
  }
  else
  {
    *arcs  = NULL;
    *narcs = 0;
  }

  return 0;
}

struct arc *
graph_find_arc_hash(struct arc_hash *hash, int end0, int end1)
{
  int t, hashval;
  struct arc *arc;

  if (end0 > end1)
    SWAP(end0, end1, t);
  if (hash->size == 0)
    return NULL;

  hashval = (end0 * hash->mult + end1) % hash->size;
  for (arc = hash->table[hashval]; arc; arc = arc->hash_next)
  {
    if (arc->tail->i == end0 && arc->head->i == end1)
    {
      return arc;
    }
  }
  return NULL;
}

void
graph_free_arc_hash(struct arc_hash **hash)
{
  free((*hash)->table);
  free((*hash));
}
