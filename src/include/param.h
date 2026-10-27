#ifndef PARAM_H_
#define PARAM_H_
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#define MAX_FILE_DIST_NAME 32
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
typedef struct param {
  uint32_t num_cli,num_dts,stripe_width,rng_seed,sample_size;
  uint64_t data_size,stripe_size;
  bool is_shared,is_quiet,explicit_data_size,explicit_sample_size;
  char file_dist[MAX_FILE_DIST_NAME];
  char out_path[PATH_MAX],workload_path[PATH_MAX],results_dir[PATH_MAX];
} param;
int parse_uint64(const char *,uint64_t *);
int param_parse(int,char **,param *);
int param_validate(param);
char *param_tostr(param);
#endif
