#include "screen.h"
#include "memalloc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
void screen(char *content) { puts(content); }
void screenf(char *format,...)
{
  va_list args,copy;
  va_start(args,format);va_copy(copy,args);
  int len=vsnprintf(NULL,0,format,copy);va_end(copy);
  if(len>=0) {
    char *content=malloc_assert((size_t)len+1);
    vsnprintf(content,(size_t)len+1,format,args);screen(content);free(content);
  }
  va_end(args);
}
