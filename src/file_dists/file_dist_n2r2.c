#include "file_dist.h"
#include "memalloc.h"
static uint32_t *select_dts(file_dist_context *ctx, uint32_t fid)
{
  uint32_t *selected=calloc_assert(ctx->stripe_width,sizeof(*selected));
  if (!ctx->baseline_set) {
    ctx->baseline=tinymt32_generate_uint32(&ctx->rng)%ctx->num_dts;
    ctx->baseline_set=true;
  }
  uint32_t stride=ctx->num_dts/ctx->num_files;
  if (!stride) stride=1;
  uint32_t next=(ctx->baseline+(uint64_t)fid*stride)%ctx->num_dts;
  for (uint32_t i=0; i<ctx->stripe_width; ++i) {
    selected[i]=next;
    next=(next+1u)%ctx->num_dts;
  }
  return selected;
}
const file_dist fd_n2r2={FD_N2R2,select_dts};
