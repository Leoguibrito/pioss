#include "dts.h"
#include "memalloc.h"
#include "logger.h"
#include <stdlib.h>
static dts_results state;
void dts_init(uint32_t n)
{
  state.num_dts=n;state.bytes_stored=calloc_assert(n,sizeof(uint64_t));
}
void dts_store(uint32_t server,uint64_t bytes)
{
  if(server>=state.num_dts || bytes>UINT64_MAX-state.bytes_stored[server]) {
    pioss_log(ERROR,"Invalid DTS index or byte overflow");exit(EXIT_FAILURE);
  }
  state.bytes_stored[server]+=bytes;
}
uint64_t dts_load(uint32_t server) { return state.bytes_stored[server]; }
void dts_clean(void) { free(state.bytes_stored);state.bytes_stored=NULL; }
dts_results *dts_get_results(void)
{
  dts_results *r=malloc_assert(sizeof(*r));*r=state;return r;
}
