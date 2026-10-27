#include "logger.h"
#include "memalloc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
static const char *TAG_STRING[]={FOREACH_TAG(GENERATE_STRING)};
void pioss_log(enum TAG_ENUM tag,const char *message)
{
  fprintf(tag==ERROR?stderr:stdout,"[%s] %s\n",TAG_STRING[tag],message);
}
void pioss_logf(enum TAG_ENUM tag,const char *format,...)
{
  va_list args,copy;
  va_start(args,format);va_copy(copy,args);
  int len=vsnprintf(NULL,0,format,copy);va_end(copy);
  if(len>=0) {
    char *message=malloc_assert((size_t)len+1);
    vsnprintf(message,(size_t)len+1,format,args);
    pioss_log(tag,message);free(message);
  }
  va_end(args);
}
