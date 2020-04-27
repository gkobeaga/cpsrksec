#include "../cpsrksec.h"

struct cp_par *
cp_create_params(void)
{
  struct cp_par *par       = malloc(sizeof(struct cp_par));
  par->tm_start     = 0;
  par->sec_sep      = CP_SEC_HONG;
  par->srk_rule     = CP_SRK_C1;
  par->srk_s2       = 1;
  par->srk_s3       = 1;
  par->srk_extra    = 1;
  par->sec_max_vout = 1;
  par->sec_max_vin  = 1;
  par->count_c1     = 0;
  par->count_c2     = 0;
  par->count_c3     = 0;
  par->count_s1     = 0;
  par->count_s2     = 0;
  par->count_extra  = 0;
  par->count_queue  = 0;
  return par;
}

void
cp_free_params(struct cp_par **par)
{
  if (*par)
  {
    free((*par));
    *par = NULL;
  }
}
