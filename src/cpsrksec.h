#ifndef CPSRKSEC_H
#define CPSRKSEC_H

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BIGINT (1 << 30)
#define MAXDOUBLE (1e32)

// The precision should be greater than the input data precision.
#define ONEMINUS (0.99999)
#define ZEROPLUS (0.00001)

#define SWAP(a, b, t) (((t) = (a)), ((a) = (b)), ((b) = (t)))

#define check_rval(rval, msg, label)                                            \
  {                                                                             \
    if ((rval))                                                                 \
    {                                                                           \
      fprintf(stderr, "%s\n", (msg));                                           \
      goto label;                                                               \
    }                                                                           \
  }

#define check_null(item, msg, label)                                            \
  {                                                                             \
    if ((!item))                                                                \
    {                                                                           \
      fprintf(stderr, "%s\n", (msg));                                           \
      rval = 1;                                                                 \
      goto label;                                                               \
    }                                                                           \
  }

#define realloc_scale(ptr, pnnum, count, scale)                                 \
  {                                                                             \
    __typeof__(ptr) p;                                                          \
    int newsize = (int)(((double)pnnum) * (scale));                             \
    newsize     = (newsize < pnnum + 1000 ? (pnnum + 1000) : newsize);          \
    newsize     = (newsize < count ? (count) : newsize);                        \
    p     = ((__typeof__(ptr))realloc(ptr, newsize * sizeof(__typeof__(ptr)))); \
    pnnum = ((p) ? newsize : 0);                                                \
    ptr   = p;                                                                  \
  }

#define NV_MAX 1000000
#define NA_MAX 6000000

struct arc_hash
{
  struct arc **table;
  unsigned int size;
  unsigned int mult;
};

struct graph
{
  int nv_space;
  int nv;
  int na_space;
  int na;
  int marker;
  int connected;
  struct graph *shrunk;
  struct arc_hash *archash;
  struct vertex **v;
  struct arc **arcs;
  struct graph *orig;

  // Number of non-null vertices
  int n3v;

  struct vertex *tail;
  struct vertex *head;

  int original_ncount;
  int original_ecount;
};

struct vertex
{
  int i;
  int ind;
  int fixed;
  int branch;
  int deg;
  int mark;
  int mark_aux;
  double obj;
  double coef;
  double y;
  struct vertex *shrunk;

  int comp;
  struct arc *in;
  struct arc *out;
  struct arc *edge;

  struct vertex *next;
  struct vertex *prev;

  // SRK
  struct vertex *members;
  int nmembers;
  struct vertex *orig;
  struct vertex *max;
  struct vertex *parent;

  // SRK rules (C2, C3)
  struct vertex *qnext;
  double vt_x;
  double ut_x;
  int onqueue;

  // Maxflow
  double excess;
  int active;
  int label;
  struct vertex *search_next;
  struct vertex *high_next;
  struct vertex *level_prev;
  struct vertex *level_next;
  struct arc *ecurrent;
};

struct arc
{
  struct vertex *tail;
  struct vertex *head;

  struct arc *prev;
  struct arc *next;

  struct arc *t_prev;
  struct arc *t_next;
  struct arc *h_prev;
  struct arc *h_next;

  struct arc *hash_prev;
  struct arc *hash_next;

  int ind;
  int i;

  double obj;
  double cost;

  int fixed;
  int branch;

  double x;
  double coef;

  double flow;
};

struct graph *
graph_create(void);
void
graph_erase(struct graph *graph),
graph_free(struct graph *graph);
int
graph_copy(struct graph *ingraph, struct graph *outgraph);

/* Used to break ties when reordering
 * 0 Random
 * 1 Maintain graph order
 * 2 Maintain previous order  */
#define REORDER 0

int
graph_add_vertices(struct graph *graph, int nadd),
graph_reorder_vertices(struct graph *graph);
void
graph_del_vertices(struct graph *graph, int ndel, const int num[]);

struct arc *
graph_add_arc(struct graph *graph, int i, int j);
struct arc *
graph_find_arc(struct graph *graph, struct vertex *tail, struct vertex *head);
void
graph_del_arc(struct graph *graph, struct arc **a);

void
graph_identify_vertices(struct graph *graph, struct vertex *v, struct vertex *u);
int
graph_expand_vertex(struct graph *graph, struct vertex *srkv, int *vcount,
                    struct vertex ***verts);

int
graph_init_arc_hash(struct graph *graph, int size),
graph_del_arc_hash(struct arc_hash *hash, struct arc *arc),
graph_getall_arc_hash(struct arc_hash *hash, int *narcs, struct arc ***arcs);
void
graph_add_arc_hash(struct arc_hash *hash, struct arc *arc),
graph_free_arc_hash(struct arc_hash **hash);
struct arc *
graph_find_arc_hash(struct arc_hash *hash, int end0, int end1);

#define otherend(arc, v) ((arc)->tail->i == (v->i) ? (arc)->head : (arc)->tail)
#define outnext(arc, v)                                                         \
  ((arc)->tail->i == (v->i) ? (arc)->t_next : (arc)->h_next)
#define outprev(arc, v)                                                         \
  ((arc)->tail->i == (v->i) ? (arc)->t_prev : (arc)->h_prev)

struct segment
{
  int lo;
  int hi;
};

struct clique
{
  int i;
  int nodecount;
  int segcount;
  struct segment *nodes;
  int hashnext;
  int refcount;
  double val;
  struct clique *prev;
  struct clique *next;
};

#define FOREACH_NODE_IN_CLIQUE(i, c, tmp)                                       \
  for (tmp = 0; tmp < c->segcount; tmp++)                                       \
    for (i = c->nodes[tmp].lo; i <= c->nodes[tmp].hi; i++)

struct clique *
clique_create(void),
*conv_vertices2clique(struct graph *graph, struct vertex **vert, int acount),
*conv_vertices2coclique(struct graph *graph, struct vertex **vert, int acount);
int
clique_copy(struct clique *in, struct clique *out),
clique_count(struct clique *clique);
void
clique_free(struct clique *clique),
clique_print(struct clique *clique);

struct repo
{
  int size;
  int cliquespace;
  int cliquehashsize;
  int cliquefree;
  int *cliquehash;
  struct clique **cliques;
  int tot_n;
};

struct repo *
repo_clique_create(struct graph *graph);
int
repo_clique_init(struct repo *repo, int size),
clique_eq(struct clique *c, struct clique *d),
repo_clique_register(struct graph *graph, struct repo *repo, struct clique *c);
unsigned int
clique_hash(struct clique *c);
void
repo_unregister_clique(struct graph *graph, struct repo *repo, int c),
repo_clique_free(struct repo **repo);

void
graph_print(struct graph *graph),
graph_write(struct graph *graph);
struct graph *
graph_read(char *fname);

struct cp_par
{
  double tm_start;
  double tm_end;
  double tm_lim;
  int sec_sep;
#define CP_SEC_NONE 0
#define CP_SEC_HONG 1
#define CP_SEC_GOMORYHU 2
  int srk_rule;
#define CP_SRK_NONE 0
#define CP_SRK_C1 1
#define CP_SRK_C1C2 2
#define CP_SRK_C1C2C3 3
#define CP_SRK_S1 4
  int srk_s2;
  int srk_s3;
  int srk_extra;
  int sec_max_vout;
  int sec_max_vin;
  int count_c1;
  int count_c2;
  int count_c3;
  int count_s1;
  int count_s2;
  int count_extra;
  int count_queue;
  const char *stats_file;
};

#define C2_ZEROPLUS 0.00001

struct cp_par *
cp_create_params(void);

void
cp_free_params(struct cp_par **par);

int
cp_shrink_graph(struct cp_par *par, struct graph *graph, struct vertex *qstart,
                struct repo *repo),
cp_shrink_c1(struct cp_par *par, struct graph *graph, struct vertex *qstart,
             struct repo *repo),
cp_shrink_c1c2(struct cp_par *par, struct graph *graph, struct vertex *qstart,
               struct repo *repo),
cp_shrink_s1(struct cp_par *par, struct graph *graph, struct vertex *qstart,
             struct repo *repo),
cp_shrink_c1c2c3(struct cp_par *par, struct graph *graph, struct vertex *qstart,
                 struct repo *repo),
cp_shrink_s1c3(struct cp_par *par, struct graph *graph, struct vertex *qstart,
               struct repo *repo);

int
cp_sec_hong(struct cp_par *par, struct graph *graph, struct repo *repo),
cp_sec_gomoryhu(struct cp_par *par, struct graph *graph, struct repo *repo),
cp_sec_sep(struct cp_par *par, struct graph *graph, struct repo *repo);

#define ADD_TO_SRK_QUEUE(n)                                                     \
  {                                                                             \
    if (!(n)->onqueue)                                                          \
    {                                                                           \
      (n)->qnext = NULL;                                                        \
      if (qtail)                                                                \
        qtail->qnext = (n);                                                     \
      else                                                                      \
        qhead = (n);                                                            \
      qtail        = (n);                                                       \
      (n)->onqueue = 1;                                                         \
    }                                                                           \
  }

int
repo_cp_save_vertices(struct graph *graph, struct vertex *u, struct vertex *v,
                      double eweight, struct repo *repo);

struct cpcut
{
  int hcount;
  struct clique **handles;
  struct clique **teeth;
  int tcount;

  int *verts;
  double vycoef;

  struct clique **cliques;

  double rhs;
  char sense;

  struct cpcut *next;
  struct cpcut *prev;
};

struct cpcut *
cp_create_cut(void);
double
cp_eval_cut(struct graph *graph, struct cpcut *cut);
void
cp_get_cut_arcs(struct graph *graph, struct cpcut *cut, int *nzcnt,
                struct arc **nzlist);
void
cp_print_cut(struct cpcut *cut),
cp_free_cut(struct cpcut **cut);

int
repo_clique_get_cuts(struct cp_par *par, struct graph *graph, struct repo *repo,
                     int *cutcount, struct cpcut **cuts);

int
mincut_solve(struct graph *graph, struct vertex *s, struct vertex *t,
             double *value, struct vertex ***verts, int *vcount);
double
maxflow_solve(struct graph *graph, struct vertex *s, struct vertex *t);

struct ghnode
{
  int i;
  int mark;
  struct ghnode *parent;
  struct ghnode *next_sibling;
  struct ghnode *prev_sibling;
  struct ghnode *child;
  double cutval;
  int nchild;
  int ndesc;
  struct vertex *in_max;
  struct vertex *out_max;
  struct vertex *special;
  int mcount;
  struct ghnode **members;
  struct vertex *pseudonode;
  struct ghnode *next;
};

struct ghtree
{
  int nn;
  struct ghnode **n;
  struct ghnode *root;
  int marker;
};

struct ghtree *
ghtree_create(void);
struct ghnode *
ghtree_add_node(struct ghtree *ghtree);
void
ghtree_add_nodes(struct ghtree *ghtree, int nn),
ghtree_print(struct ghtree *ghtree), ghtree_free(struct ghtree *ghtree);

int
ghtree_get(struct cp_par *par, struct graph *graph, int rootind,
           struct ghtree **ghtree);

unsigned int
prime_next(unsigned int x);

#endif
