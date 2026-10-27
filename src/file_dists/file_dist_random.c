#include "file_dist.h"
#include "memalloc.h"
#include <stdlib.h>
static uint32_t *select_dts(file_dist_context *ctx, uint32_t fid)
{
  (void)fid;
  uint32_t *pool = calloc_assert(ctx->num_dts, sizeof(*pool));
  uint32_t *selected = calloc_assert(ctx->stripe_width, sizeof(*selected));
  for (uint32_t i=0; i<ctx->num_dts; ++i) pool[i]=i;
  /* Partial Fisher-Yates, with unbiased bounded draws. */
  for (uint32_t i=0; i<ctx->stripe_width; ++i) {
    uint32_t j=i+rng_bounded(&ctx->rng,ctx->num_dts-i);
    uint32_t tmp=pool[i]; pool[i]=pool[j]; pool[j]=tmp;
    selected[i]=pool[i];
  }
  free(pool);
  return selected;
}
const file_dist fd_random={FD_RANDOM,select_dts};
