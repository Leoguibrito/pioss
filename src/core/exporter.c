#include "exporter.h"
#include "csvexporter.h"
#include "logger.h"
#include "timing.h"
#include "version.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int export_results(const dts_results *r,param p)
{
  struct stat st;
  char path[PATH_MAX];
  int status=stat(p.out_path,&st);
  if(status==0 && S_ISDIR(st.st_mode)) {
    int n=snprintf(path,sizeof(path),"%s/n%"PRIu32"_m%"PRIu32"_w%"PRIu32"_%s-%.6f.csv",
                   p.out_path,p.num_cli,p.num_dts,p.stripe_width,p.file_dist,usectime());
    if(n<0 || (size_t)n>=sizeof(path)) return 1;
    return export_csv(r,path);
  }
  if(status && errno!=ENOENT) return 1;
  return export_csv(r,p.out_path);
}
static FILE *open_child(const char *dir,const char *name)
{
  char path[PATH_MAX];
  int n=snprintf(path,sizeof(path),"%s/%s",dir,name);
  if(n<0 || (size_t)n>=sizeof(path)) return NULL;
  return fopen(path,"wx");
}
static void json_string(FILE *f,const char *s)
{
  fputc('"',f);
  for(const unsigned char *p=(const unsigned char *)s;*p;++p) {
    if(*p=='"' || *p=='\\') { fputc('\\',f);fputc(*p,f); }
    else if(*p<32) fprintf(f,"\\u%04x",*p);
    else fputc(*p,f);
  }
  fputc('"',f);
}
static int close_file(FILE *f)
{
  int error=ferror(f);
  if(fclose(f)) error=1;
  return error;
}
int export_bundle(const dts_results *r,param p,const workload *w,
                  const file_record *records,simulation_metrics m)
{
  if(mkdir(p.results_dir,0777)) {
    pioss_logf(ERROR,"Cannot create NEW results directory: %s",p.results_dir);return 1;
  }
  FILE *f=open_child(p.results_dir,"servers.csv");
  if(!f) goto fail;
  fputs("DTS,Bytes\n",f);
  for(uint32_t i=0;i<r->num_dts;++i) fprintf(f,"%"PRIu32",%"PRIu64"\n",i,r->bytes_stored[i]);
  if(close_file(f)) goto fail;
  f=open_child(p.results_dir,"files.csv");
  if(!f) goto fail;
  fputs("client,fid,size_bytes,destinations,queries,distinct_queried\n",f);
  for(uint32_t i=0;i<p.num_cli;++i) {
    const file_record *rec=&records[i];
    fprintf(f,"%"PRIu32",%"PRIu32",%"PRIu64",\"",i,rec->fid,rec->size);
    for(uint32_t j=0;j<p.stripe_width;++j)
      fprintf(f,"%s%"PRIu32,j?";":"",rec->destinations[j]);
    fprintf(f,"\",%"PRIu64",%"PRIu32"\n",rec->queries,rec->distinct);
  }
  if(close_file(f)) goto fail;
  /* Summary is written last. Runners also validate all payload files. */
  f=open_child(p.results_dir,"summary.json");
  if(!f) goto fail;
  fprintf(f,"{\n\"schema_version\":1,\n\"version\":\"%s\",\n\"revision\":\"%s\",\n"
    "\"source_sha256\":\"%s\",\n\"policy\":",PIOSS_VERSION,PIOSS_REVISION,PIOSS_SOURCE_SHA256);
  json_string(f,p.file_dist);
  fprintf(f,",\n\"num_cli\":%"PRIu32",\"num_dts\":%"PRIu32",\"stripe_width\":%"PRIu32
    ",\"stripe_size\":%"PRIu64",\"policy_seed\":%"PRIu32",\"sample_size\":%"PRIu32
    ",\"shared\":%s,\n\"workload_path\":",
    p.num_cli,p.num_dts,p.stripe_width,p.stripe_size,p.rng_seed,p.sample_size,p.is_shared?"true":"false");
  json_string(f,p.workload_path);
  fprintf(f,",\n\"workload_sha256\":\"%s\",\"total_bytes\":%"PRIu64
    ",\"workload_min\":%"PRIu64",\"workload_max\":%"PRIu64",\"workload_mean\":%.17g,"
    "\"workload_cv\":%.17g,\n\"total_queries\":%"PRIu64",\"queries_per_file\":%.17g,"
    "\"total_distinct_queried\":%"PRIu64",\"distinct_queried_per_file\":%.17g,"
    "\"load_cv\":%.17g,\"max_mean_ratio\":%.17g\n}\n",
    w->sha256,w->total,w->min,w->max,w->mean,w->cv,m.total_queries,
    (double)m.total_queries/p.num_cli,m.total_distinct,(double)m.total_distinct/p.num_cli,
    m.load_cv,m.max_mean_ratio);
  if(close_file(f)) goto fail;
  return 0;
fail:
  pioss_logf(ERROR,"Incomplete result bundle: %s",p.results_dir);
  return 1;
}
