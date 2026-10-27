#ifndef WORKLOAD_H_
#define WORKLOAD_H_
#include "param.h"
typedef struct {
  uint64_t *sizes,total,min,max;
  double mean,cv;
  char sha256[65];
} workload;
int workload_load(param,workload *);
void workload_clean(workload *);
#endif
