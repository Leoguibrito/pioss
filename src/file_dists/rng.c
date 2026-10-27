#include "file_dist.h"
#include <assert.h>
void rng_seed(tinymt32_t *rng, uint32_t seed)
{
  /* Preserve the original PIOSS TinyMT parameterization. */
  *rng = (tinymt32_t){0};
  tinymt32_init(rng, seed);
}
uint32_t rng_bounded(tinymt32_t *rng, uint32_t bound)
{
  assert(bound);
  uint32_t threshold = (uint32_t)(0u - bound) % bound, value;
  do { value = tinymt32_generate_uint32(rng); } while (value < threshold);
  return value % bound;
}
