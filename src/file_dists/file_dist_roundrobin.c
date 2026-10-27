#include "file_dist.h"
#include "memalloc.h"
static uint32_t *select_dts(file_dist_context *ctx, uint32_t fid)
{
  (void)fid;
  uint32_t *selected=calloc_assert(ctx->stripe_width,sizeof(*selected));
  /* Historical PIOSS: a fresh random start for EACH file. */
  uint32_t next=tinymt32_generate_uint32(&ctx->rng)%ctx->num_dts;
  for (uint32_t i=0; i<ctx->stripe_width; ++i) {
    selected[i]=next;
    next=(next+1u)%ctx->num_dts;
  }
  return selected;
}
const file_dist fd_roundrobin={FD_ROUNDROBIN,select_dts};
