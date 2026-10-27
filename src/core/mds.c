#include "mds.h"
#include "file_dist.h"
#include "dts.h"
#include "memalloc.h"
#include <stdlib.h>
#include <string.h>
static struct {
  const file_dist *policy;
  file_dist_context context;
  uint32_t **table;
  bool *seen;
  uint64_t queries;
  uint32_t distinct;
} state;
static uint64_t query(void *user,uint32_t server)
{
  (void)user;
  ++state.queries;
  if(!state.seen[server]) { state.seen[server]=true;++state.distinct; }
  return dts_load(server);
}
void mds_init(param p)
{
  memset(&state,0,sizeof(state));
  const file_dist *policies[]={&fd_random,&fd_roundrobin,&fd_n2r2,&fd_power,&fd_least};
  for(size_t i=0;i<sizeof(policies)/sizeof(*policies);++i)
    if(!strcmp(p.file_dist,policies[i]->name)) state.policy=policies[i];
  state.context=(file_dist_context){.num_dts=p.num_dts,.stripe_width=p.stripe_width,
      .num_files=p.is_shared?1:p.num_cli,.sample_size=p.sample_size,.query=query};
  rng_seed(&state.context.rng,p.rng_seed);
  state.table=calloc_assert(state.context.num_files,sizeof(*state.table));
  state.seen=calloc_assert(p.num_dts,sizeof(*state.seen));
}
uint32_t *mds_open(uint32_t fid)
{
  state.queries=0;state.distinct=0;
  memset(state.seen,0,state.context.num_dts*sizeof(*state.seen));
  if(!state.table[fid]) state.table[fid]=state.policy->select_dts(&state.context,fid);
  return state.table[fid];
}
void mds_counts(uint64_t *queries,uint32_t *distinct)
{
  *queries=state.queries;*distinct=state.distinct;
}
void mds_clean(void)
{
  for(uint32_t i=0;i<state.context.num_files;++i) free(state.table[i]);
  free(state.table);free(state.seen);
}
