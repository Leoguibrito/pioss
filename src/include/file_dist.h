#ifndef FILE_DIST_H_
#define FILE_DIST_H_
#include <stdbool.h>
#include <stdint.h>
#include "tinymt32.h"
#define FD_RANDOM "Random"
#define FD_ROUNDROBIN "RoundRobin"
#define FD_N2R2 "N2R2"
#define FD_POWER "PowerOfD"
#define FD_LEAST "LeastLoaded"
typedef uint64_t (*load_query)(void *, uint32_t);
typedef struct {
  uint32_t num_dts, stripe_width, num_files, sample_size;
  tinymt32_t rng;
  load_query query;
  void *query_user;
  uint32_t baseline;
  bool baseline_set;
} file_dist_context;
typedef struct {
  const char *name;
  uint32_t *(*select_dts)(file_dist_context *, uint32_t);
} file_dist;
uint32_t rng_bounded(tinymt32_t *, uint32_t);
void rng_seed(tinymt32_t *, uint32_t);
extern const file_dist fd_random, fd_roundrobin, fd_n2r2, fd_power, fd_least;
#endif
