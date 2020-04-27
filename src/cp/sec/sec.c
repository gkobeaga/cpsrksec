#include "../../cpsrksec.h"

int
cp_sec_sep(struct cp_par *par, struct graph *graph, struct repo *repo)
{
  int rval = 0;
  if (par->sec_sep == CP_SEC_HONG)
  {
    rval = cp_sec_hong(par, graph, repo);
  }
  else if (par->sec_sep == CP_SEC_GOMORYHU)
  {
    rval = cp_sec_gomoryhu(par, graph, repo);
  }
  return rval;
}
