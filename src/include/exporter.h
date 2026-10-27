#ifndef EXPORTER_H_
#define EXPORTER_H_
#include "dts.h"
#include "param.h"
#include "workload.h"
typedef struct {
  uint32_t fid;
  uint64_t size,queries;
  uint32_t distinct,*destinations;
} file_record;
typedef struct {
  uint64_t total_queries,total_distinct;
  double load_cv,max_mean_ratio;
} simulation_metrics;
int export_results(const dts_results *,param);
int export_bundle(const dts_results *,param,const workload *,const file_record *,simulation_metrics);
#endif
