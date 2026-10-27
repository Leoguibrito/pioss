#include "param.h"
#include "file_dist.h"
#include "logger.h"
#include "memalloc.h"
#include "timing.h"
#include "types.h"
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int parse_uint64(const char *text,uint64_t *value)
{
  if (!text || !*text) return 1;
  for (const char *p=text; *p; ++p) if (*p<'0' || *p>'9') return 1;
  errno=0;
  char *end;
  uintmax_t parsed=strtoumax(text,&end,10);
  if (errno || *end || parsed>UINT64_MAX) return 1;
  *value=(uint64_t)parsed;
  return 0;
}
static int copy_arg(char *dest,size_t capacity,const char *arg)
{
  if (!*arg || strlen(arg)>=capacity) return 1;
  strcpy(dest,arg);
  return 0;
}
static void usage(const char *exe)
{
  fprintf(stderr,
    "usage: %s [OPTIONS]\n"
    "  --num_cli/-n N       clients (default 1)\n"
    "  --data_size/-b B     bytes per client (default 1048576)\n"
    "  --shared/-s          shared file (legacy policies only)\n"
    "  --num_dts/-m N       data servers (default 2)\n"
    "  --stripe_width/-w W  servers per file (default 1)\n"
    "  --stripe_size/-z B   stripe fragment bytes (default 65536)\n"
    "  --file_dist/-f NAME  Random, RoundRobin, N2R2, PowerOfD, LeastLoaded\n"
    "  --rng_seed/-r SEED   policy seed (uint32; default clock-derived)\n"
    "  --workload_path P    exactly num_cli positive byte sizes, one per line\n"
    "  --sample_size D      candidates per destination; PowerOfD only\n"
    "  --out_path/-o P      legacy DTS,Bytes CSV (file or existing directory)\n"
    "  --results_dir P     new directory with CSV files and summary.json\n"
    "  --quiet/-q          suppress screen output\n"
    "  --help/-h           show this help\n",exe);
}
int param_parse(int argc,char **argv,param *p)
{
  *p=(param){.num_cli=1,.data_size=MIB,.num_dts=2,.stripe_width=1,
             .stripe_size=64*KIB,.rng_seed=(uint32_t)((uint64_t)(usectime()*1e6))};
  strcpy(p->file_dist,FD_RANDOM);
  enum { WORKLOAD=256,SAMPLE,RESULTS };
  const struct option opts[]={
    {"num_cli",1,0,'n'},{"data_size",1,0,'b'},{"shared",0,0,'s'},
    {"num_dts",1,0,'m'},{"stripe_width",1,0,'w'},{"stripe_size",1,0,'z'},
    {"file_dist",1,0,'f'},{"rng_seed",1,0,'r'},{"out_path",1,0,'o'},
    {"quiet",0,0,'q'},{"help",0,0,'h'},{"workload_path",1,0,WORKLOAD},
    {"sample_size",1,0,SAMPLE},{"results_dir",1,0,RESULTS},{0,0,0,0}};
  optind=0; opterr=0;
  int c,error=0;
  while ((c=getopt_long(argc,argv,"n:b:sm:w:z:f:r:o:qh",opts,NULL))!=-1) {
    uint64_t number=0;
    switch(c) {
    case 'n': case 'm': case 'w': case 'r': case SAMPLE:
      if (parse_uint64(optarg,&number) || number>UINT32_MAX) { error=1; break; }
      if (c=='n') p->num_cli=(uint32_t)number;
      if (c=='m') p->num_dts=(uint32_t)number;
      if (c=='w') p->stripe_width=(uint32_t)number;
      if (c=='r') p->rng_seed=(uint32_t)number;
      if (c==SAMPLE) { p->sample_size=(uint32_t)number; p->explicit_sample_size=true; }
      break;
    case 'b': p->explicit_data_size=true; error|=parse_uint64(optarg,&p->data_size); break;
    case 'z': error|=parse_uint64(optarg,&p->stripe_size); break;
    case 's': p->is_shared=true; break;
    case 'q': p->is_quiet=true; break;
    case 'f': error|=copy_arg(p->file_dist,sizeof(p->file_dist),optarg); break;
    case 'o': error|=copy_arg(p->out_path,sizeof(p->out_path),optarg); break;
    case WORKLOAD: error|=copy_arg(p->workload_path,sizeof(p->workload_path),optarg); break;
    case RESULTS: error|=copy_arg(p->results_dir,sizeof(p->results_dir),optarg); break;
    case 'h': usage(argv[0]); exit(EXIT_SUCCESS);
    default: error=1; break;
    }
  }
  if (optind<argc) error=1;
  if (error) { pioss_log(ERROR,"Invalid option or numeric argument"); usage(argv[0]); }
  return error;
}
int param_validate(param p)
{
  bool power=!strcmp(p.file_dist,FD_POWER);
  bool informed=power || !strcmp(p.file_dist,FD_LEAST);
  bool known=informed || !strcmp(p.file_dist,FD_RANDOM) ||
             !strcmp(p.file_dist,FD_ROUNDROBIN) || !strcmp(p.file_dist,FD_N2R2);
  if (!known || !p.num_cli || !p.num_dts || !p.stripe_width ||
      p.stripe_width>p.num_dts || !p.stripe_size || !p.data_size ||
      (power && (!p.explicit_sample_size || !p.sample_size || p.sample_size>p.num_dts)) ||
      (!power && p.explicit_sample_size) ||
      (p.workload_path[0] && (p.explicit_data_size || p.is_shared)) ||
      (informed && p.is_shared) || (p.out_path[0] && p.results_dir[0]) ||
      (!p.workload_path[0] && p.data_size>UINT64_MAX/p.num_cli)) {
    pioss_log(ERROR,"Invalid configuration: check positive sizes/counts, width, policy options, "
                    "workload/shared conflicts, output conflicts and total-byte overflow");
    return 1;
  }
  return 0;
}
char *param_tostr(param p)
{
  const char *format=
    "num_cli=%"PRIu32" data_size=%"PRIu64" shared=%s num_dts=%"PRIu32"\n"
    "stripe_width=%"PRIu32" stripe_size=%"PRIu64" file_dist=%s rng_seed=%"PRIu32"\n"
    "sample_size=%"PRIu32" workload_path=%s out_path=%s results_dir=%s";
  int len=snprintf(NULL,0,format,p.num_cli,p.data_size,p.is_shared?"true":"false",
      p.num_dts,p.stripe_width,p.stripe_size,p.file_dist,p.rng_seed,p.sample_size,
      p.workload_path,p.out_path,p.results_dir);
  if (len<0) return NULL;
  char *out=malloc_assert((size_t)len+1);
  snprintf(out,(size_t)len+1,format,p.num_cli,p.data_size,p.is_shared?"true":"false",
      p.num_dts,p.stripe_width,p.stripe_size,p.file_dist,p.rng_seed,p.sample_size,
      p.workload_path,p.out_path,p.results_dir);
  return out;
}
