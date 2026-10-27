#include "workload.h"
#include "logger.h"
#include "memalloc.h"
#include "sha256.h"
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int workload_load(param p,workload *w)
{
  *w=(workload){.min=UINT64_MAX};
  w->sizes=calloc_assert(p.num_cli,sizeof(*w->sizes));
  uint32_t count=0;
  if(p.workload_path[0]) {
    FILE *f=fopen(p.workload_path,"r");
    if(!f) { pioss_logf(ERROR,"Cannot read workload: %s",p.workload_path);goto fail; }
    char *line=NULL;size_t capacity=0;ssize_t length;int error=0;
    while((length=getline(&line,&capacity,f))!=-1) {
      if(memchr(line,0,(size_t)length)) { error=1;break; }
      char *start=line;
      while(*start && isspace((unsigned char)*start)) ++start;
      char *end=start+strlen(start);
      while(end>start && isspace((unsigned char)end[-1])) --end;
      *end=0;
      if(!*start) continue;
      uint64_t value;
      if(count>=p.num_cli || parse_uint64(start,&value) || !value) { error=1;break; }
      w->sizes[count++]=value;
    }
    if(ferror(f)) error=1;
    free(line);
    if(fclose(f)) error=1;
    if(error || count!=p.num_cli) {
      pioss_log(ERROR,"Workload must contain exactly num_cli positive uint64 sizes");goto fail;
    }
  } else {
    for(uint32_t i=0;i<p.num_cli;++i) w->sizes[i]=p.data_size;
  }
  sha256 hash;sha256_init(&hash);
  long double mean=0,m2=0;
  for(uint32_t i=0;i<p.num_cli;++i) {
    uint64_t size=w->sizes[i];
    if(size>UINT64_MAX-w->total) { pioss_log(ERROR,"Total-byte overflow");goto fail; }
    w->total+=size;
    if(size<w->min) w->min=size;
    if(size>w->max) w->max=size;
    long double delta=(long double)size-mean;
    mean+=delta/((long double)i+1);m2+=delta*((long double)size-mean);
    char text[32];int n=snprintf(text,sizeof(text),"%"PRIu64"\n",size);
    sha256_update(&hash,text,(size_t)n);
  }
  sha256_final(&hash,w->sha256);
  w->mean=(double)mean;w->cv=(double)(sqrtl(m2/p.num_cli)/mean);
  return 0;
fail:
  workload_clean(w);return 1;
}
void workload_clean(workload *w) { free(w->sizes);w->sizes=NULL; }
