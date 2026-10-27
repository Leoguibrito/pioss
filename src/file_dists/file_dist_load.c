#include "file_dist.h"
#include "memalloc.h"
#include <stdlib.h>
static uint32_t *power(file_dist_context *ctx, uint32_t fid)
{
  (void)fid;
  uint32_t *pool=calloc_assert(ctx->num_dts,sizeof(*pool));
  uint32_t *selected=calloc_assert(ctx->stripe_width,sizeof(*selected));
  for (uint32_t i=0; i<ctx->num_dts; ++i) pool[i]=i;
  uint32_t eligible=ctx->num_dts;
  for (uint32_t slot=0; slot<ctx->stripe_width; ++slot) {
    uint32_t count=ctx->sample_size<eligible?ctx->sample_size:eligible;
    uint32_t winner=0,ties=0;
    uint64_t best=UINT64_MAX;
    for (uint32_t i=0; i<count; ++i) {
      uint32_t j=i+rng_bounded(&ctx->rng,eligible-i);
      uint32_t tmp=pool[i]; pool[i]=pool[j]; pool[j]=tmp;
      uint64_t load=ctx->query(ctx->query_user,pool[i]);
      if (load<best) { best=load; winner=i; ties=1; }
      else if (load==best) {
        ++ties;
        if (rng_bounded(&ctx->rng,ties)==0) winner=i;
      }
    }
    selected[slot]=pool[winner];
    pool[winner]=pool[--eligible];
  }
  free(pool);
  return selected;
}
typedef struct { uint64_t load; uint32_t server,tie_order; } ranked;
static int compare(const void *a,const void *b)
{
  const ranked *x=a,*y=b;
  if (x->load!=y->load) return x->load<y->load?-1:1;
  return (x->tie_order>y->tie_order)-(x->tie_order<y->tie_order);
}
static uint32_t *least(file_dist_context *ctx,uint32_t fid)
{
  (void)fid;
  ranked *ranks=calloc_assert(ctx->num_dts,sizeof(*ranks));
  uint32_t *selected=calloc_assert(ctx->stripe_width,sizeof(*selected));
  for (uint32_t i=0; i<ctx->num_dts; ++i) {
    ranks[i].server=i; ranks[i].load=ctx->query(ctx->query_user,i);
  }
  for (uint32_t i=0; i<ctx->num_dts; ++i) {
    uint32_t j=i+rng_bounded(&ctx->rng,ctx->num_dts-i);
    ranked tmp=ranks[i]; ranks[i]=ranks[j]; ranks[j]=tmp;
    ranks[i].tie_order=i;
  }
  qsort(ranks,ctx->num_dts,sizeof(*ranks),compare);
  for (uint32_t i=0; i<ctx->stripe_width; ++i) selected[i]=ranks[i].server;
  free(ranks);
  return selected;
}
const file_dist fd_power={FD_POWER,power};
const file_dist fd_least={FD_LEAST,least};
