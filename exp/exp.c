#include "../src/cpsrksec.h"

//#define RNG_SEED (2020)

static void
help_print_general(const char *my_name),
help_print_hong(const char *my_name), help_print_gomoryhu(const char *my_name);

static int
get_params(int argc, char *argv[], struct cp_par *par, char **instance,
           char **shortname);

int
main(int argc, char *argv[])
{
  int rval = 0;
  struct cp_par *par;

  struct graph *supportgraph = NULL;
  struct graph *srkgraph     = NULL;

  char *instance = NULL, *shortname = NULL;
  FILE *file   = NULL;
  int nsec     = 0;
  int ninvalid = 0;
  int nvalid   = 0;

  double val;
  double maxval         = -20000000000;
  struct vertex **verts = NULL;

  int cutcount       = 0;
  struct cpcut *cuts = NULL;
  struct cpcut *cut;

  struct vertex **nodevector = NULL;
  struct repo *repo          = NULL;

  struct timespec start, pre_stop, sep_stop, cut_stop, tm_seed;

  par = cp_create_params();

#ifdef RNG_SEED
  srand(RNG_SEED);
#else
  clock_gettime(CLOCK_REALTIME, &tm_seed);
  srand(tm_seed.tv_nsec / 1000);
#endif

  rval = get_params(argc, argv, par, &instance, &shortname);
  check_rval(rval, "invalid arguments", CLEANUP);

  // 0. Read input data
  supportgraph = graph_read(instance);
  check_null(supportgraph, "graph_create failed", CLEANUP);

  file = fopen("exp-results.csv", "a");
  check_null(file, "Unable to create file", CLEANUP);

  unsigned long flen = (unsigned long)ftell(file);
  if (flen == 0)
    fprintf(file,
            "name,nv,gnv,gna,strat,s2,s3,extra,gomoryhu,max_in,max_"
            "out,snv,sna,pre_count_c1,pre_count_c2,pre_count_c3,pre_"
            "count_s1,pre_count_s2,pre_count_queue,pre_count_qsets,pre_time,"
            "sep_count_c1,sep_count_c2,sep_count_c3,sep_count_s1,sep_count_s2,"
            "sep_count_extra,sep_count_queue,sep_count_qsets,sep_time,cut_"
            "time,nsec,maxval,valid,invalid\n");
  fprintf(file, "%s,", shortname);
  fprintf(file, "%d,", supportgraph->nv);

  fprintf(file, "%d,", supportgraph->n3v);
  fprintf(file, "%d,", supportgraph->na);

  fprintf(file, "%d,", par->srk_rule);
  fprintf(file, "%d,", par->srk_s2);
  fprintf(file, "%d,", par->srk_s3);
  fprintf(file, "%d,", par->srk_extra);
  if (par->sec_sep == CP_SEC_HONG)
    fprintf(file, "0,");
  else if (par->sec_sep == CP_SEC_GOMORYHU)
    fprintf(file, "1,");
  fprintf(file, "%d,", par->sec_max_vin);
  fprintf(file, "%d,", par->sec_max_vout);

  // 1. Pre-process: shrinking
  clock_gettime(CLOCK_REALTIME, &start);

  srkgraph = supportgraph->shrunk = graph_create();
  rval                            = graph_copy(supportgraph, srkgraph);
  check_rval(rval, "conv_graph0srkgraph failed", CLEANUP);

  repo = repo_clique_create(supportgraph);

  graph_reorder_vertices(srkgraph);

  if (par->srk_rule != CP_SRK_NONE)
  {
    rval = cp_shrink_graph(par, srkgraph, NULL, repo);
    check_rval(rval, "shrink_cp_c1 failed", CLEANUP);
  }

  clock_gettime(CLOCK_REALTIME, &pre_stop);

  fprintf(file, "%d,", srkgraph->n3v);
  fprintf(file, "%d,", srkgraph->na);
  fprintf(file, "%d,", par->count_c1);
  fprintf(file, "%d,", par->count_c2);
  fprintf(file, "%d,", par->count_c3);
  fprintf(file, "%d,", par->count_s1);
  fprintf(file, "%d,", par->count_s2);
  fprintf(file, "%d,", par->count_queue);
  fprintf(file, "%d,", repo->size);
  fprintf(file, "%ld,",
          (long)((pre_stop.tv_sec - start.tv_sec) * 1000000 +
                 (pre_stop.tv_nsec - start.tv_nsec) / 1000));

  // 2. SEC separation
  cp_sec_sep(par, srkgraph, repo);
  clock_gettime(CLOCK_REALTIME, &sep_stop);

  fprintf(file, "%d,", par->count_c1);
  fprintf(file, "%d,", par->count_c2);
  fprintf(file, "%d,", par->count_c3);
  fprintf(file, "%d,", par->count_s1);
  fprintf(file, "%d,", par->count_s2);
  fprintf(file, "%d,", par->count_extra);
  fprintf(file, "%d,", par->count_queue);
  fprintf(file, "%d,", repo->size);
  fprintf(file, "%ld,",
          (long)((sep_stop.tv_sec - start.tv_sec) * 1000000 +
                 (sep_stop.tv_nsec - start.tv_nsec) / 1000));

  // 3. Cut generation
  repo_clique_get_cuts(par, supportgraph, repo, &cutcount, &cuts);
  clock_gettime(CLOCK_REALTIME, &cut_stop);

  fprintf(file, "%ld,",
          (long)((cut_stop.tv_sec - start.tv_sec) * 1000000 +
                 (cut_stop.tv_nsec - start.tv_nsec) / 1000));

  // 4. Check
  // This part is to check that algorithm works as expected.
  // It is not considered in the computation of the time.
  maxval = 0;
  while (cuts)
  {
    cut  = cuts;
    cuts = cut->next;
    val  = -cp_eval_cut(supportgraph, cut);
    nsec++;
    if (val > maxval)
      maxval = val;
    if (val <= 0)
      ninvalid++;
    else
      nvalid++;
    cp_free_cut(&cut);
  }

  fprintf(file, "%d,", nsec);
  fprintf(file, "%.3f,", maxval);
  fprintf(file, "%d,", nvalid);
  fprintf(file, "%d\n", ninvalid);

CLEANUP:
  if (repo)
    repo_clique_free(&repo);
  if (nodevector)
    free(nodevector);
  if (verts)
    free(verts);
  if (file != NULL)
    fclose(file);
  if (supportgraph)
  {
    graph_free(supportgraph);
  }
  cp_free_params(&par);

  return rval;
}

static int
get_params(int argc, char *argv[], struct cp_par *par, char **instance,
           char **shortname)
{
  char *tempfname = NULL, *str = NULL;
  if (argc == 1)
  {
    help_print_general(argv[0]);
    return 0;
  }
  else if (argc == 2 && !strcmp(argv[1], "-h"))
  {
    help_print_general(argv[0]);
    return 0;
  }
  if (strcmp(argv[1], "--hong") && strcmp(argv[1], "--gomoryhu"))
  {
    help_print_general(argv[0]);
    return 0;
  }
  if (!strcmp(argv[1], "--hong"))
  {
    if (argc == 2)
    {
      help_print_hong(argv[0]);
      return 0;
    }
    if (argc == 3 && !strcmp(argv[2], "-h"))
    {
      help_print_hong(argv[0]);
      return 0;
    }
    par->sec_sep = CP_SEC_HONG;
    if (argc != 8)
    {
      printf("invalid number of arguments\n");
      help_print_hong(argv[0]);
      return -1;
    }
    else
    {
      par->srk_rule     = atoi(argv[2]);
      par->srk_s2       = atoi(argv[3]);
      par->srk_s3       = atoi(argv[4]);
      par->srk_extra    = atoi(argv[5]);
      par->sec_max_vout = par->sec_max_vin = atoi(argv[6]);
      if (par->srk_rule != CP_SRK_S1 && par->srk_s2)
      {
        help_print_hong(argv[0]);
        return -1;
      }
      if (!par->srk_s3 && par->srk_extra)
      {
        help_print_hong(argv[0]);
        return -1;
      }
      str = strdup(argv[7]);
      while ((tempfname = strsep(&str, "/"))) *shortname = tempfname;
      *instance = argv[7];
    }
  }
  else if (!strcmp(argv[1], "--gomoryhu"))
  {
    if (argc == 2)
    {
      help_print_gomoryhu(argv[0]);
      return 0;
    }
    else if (argc == 3 && !strcmp(argv[2], "-h"))
    {
      help_print_gomoryhu(argv[0]);
      return 0;
    }
    par->sec_sep = CP_SEC_GOMORYHU;
    if (argc != 6)
    {
      printf("invalid number of arguments\n");
      help_print_gomoryhu(argv[0]);
      return -1;
    }
    else
    {
      par->srk_rule     = atoi(argv[2]);
      par->srk_s2       = atoi(argv[3]);
      par->srk_s3       = 0;
      par->srk_extra    = 0;
      par->sec_max_vout = par->sec_max_vin = atoi(argv[4]);
      if (par->srk_rule != CP_SRK_S1 && par->srk_s2)
      {
        help_print_gomoryhu(argv[0]);
        return -1;
      }
      if (!par->srk_s3 && par->srk_extra)
      {
        help_print_gomoryhu(argv[0]);
        return -1;
      }
      str = strdup(argv[5]);
      while ((tempfname = strsep(&str, "/"))) *shortname = tempfname;
      *instance = argv[5];
    }
  }
  return 0;
}

static void
help_print_general(const char *my_name)
{ /* print help information */
  printf("Usage: %s SEP -h\n", my_name);
  printf("\n");
  printf("Options:\n");
  printf("  -h              Print usage\n");
  printf("Arguments:\n");
  printf("  SEP             --hong or --gomoryhu\n");
}

static void
help_print_gomoryhu(const char *my_name)
{ /* print help information */
  printf("Usage: %s --gomoryhu RULE SECII MAX filename\n", my_name);
  printf("\n");
  printf("Options:\n");
  printf("  -h              Print usage\n");
  printf("Arguments:\n");
  printf("  RULE            Integer\n");
  printf("  SECII           Boolean \n");
  printf("  MAX             Integer (MAX,MAX) when generating cuts \n");
}

static void
help_print_hong(const char *my_name)
{ /* print help information */
  printf("Usage: %s --hong RULE S2 S3 EXTRA MAX filename\n", my_name);
  printf("\n");
  printf("Options:\n");
  printf("  -h              Print usage\n");
  printf("Arguments:\n");
  printf("  RULE            Integer\n");
  printf("  S2              Boolean \n");
  printf("  S3              Boolean \n");
  printf("  EXTRA           Boolean \n");
  printf("  MAX             Integer (MAX,MAX) when generating cuts \n");
}
