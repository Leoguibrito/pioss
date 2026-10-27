#include "csvexporter.h"
#include "logger.h"
#include <inttypes.h>
#include <stdio.h>
int export_csv(const dts_results *r,const char *path)
{
  FILE *f=fopen(path,"wx");
  if(!f) { pioss_logf(ERROR,"Cannot create new CSV: %s",path);return 1; }
  fputs("DTS,Bytes\n",f);
  for(uint32_t i=0;i<r->num_dts;++i)
    fprintf(f,"%"PRIu32",%"PRIu64"\n",i,r->bytes_stored[i]);
  int error=ferror(f);
  if(fclose(f)) error=1;
  if(error) pioss_logf(ERROR,"Failed writing CSV: %s",path);
  return error;
}
