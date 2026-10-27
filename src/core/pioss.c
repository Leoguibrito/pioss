#include "pioss.h"
#include "cli.h"
#include "dts.h"
#include "exporter.h"
#include "logger.h"
#include "mds.h"
#include "memalloc.h"
#include "screen.h"
#include "workload.h"
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
int pioss_exec(param p)
{
  workload w;
  if(workload_load(p,&w)) return 1;
  dts_init(p.num_dts);mds_init(p);
  file_record *records=calloc_assert(p.num_cli,sizeof(*records));
  simulation_metrics metrics={0};int error=0;
  for(uint32_t i=0;i<p.num_cli;++i) {
    records[i].fid=p.is_shared?0:i;
    records[i].size=w.sizes[i];
    records[i].destinations=cli_exec(records[i].fid,w.sizes[i],p.stripe_size,p.stripe_width);
    mds_counts(&records[i].queries,&records[i].distinct);
    if(records[i].queries>UINT64_MAX-metrics.total_queries ||
       records[i].distinct>UINT64_MAX-metrics.total_distinct) {
      pioss_log(ERROR,"Query-count overflow");error=1;break;
    }
    metrics.total_queries+=records[i].queries;
    metrics.total_distinct+=records[i].distinct;
  }
  dts_results *r=dts_get_results();
  long double mean=(long double)w.total/p.num_dts,variance=0;
  uint64_t maximum=0,total=0;
  for(uint32_t i=0;i<p.num_dts;++i) {
    long double delta=(long double)r->bytes_stored[i]-mean;
    variance+=delta*delta;
    if(r->bytes_stored[i]>maximum) maximum=r->bytes_stored[i];
    total+=r->bytes_stored[i];
  }
  if(total!=w.total) { pioss_log(ERROR,"Byte conservation failed");error=1; }
  metrics.load_cv=(double)(sqrtl(variance/p.num_dts)/mean);
  metrics.max_mean_ratio=(double)((long double)maximum/mean);
  if(!error && !p.is_quiet) {
    screen("DTS,Bytes");
    for(uint32_t i=0;i<p.num_dts;++i) screenf("%"PRIu32",%"PRIu64,i,r->bytes_stored[i]);
    screenf("CV=%.9g max/mean=%.9g queries=%"PRIu64,
            metrics.load_cv,metrics.max_mean_ratio,metrics.total_queries);
  }
  if(!error && p.out_path[0]) error=export_results(r,p);
  if(!error && p.results_dir[0]) error=export_bundle(r,p,&w,records,metrics);
  free(r);free(records);mds_clean();dts_clean();workload_clean(&w);
  return error;
}
