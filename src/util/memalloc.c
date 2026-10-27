#include "memalloc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static void fail(void) { fputs("PIOSS: allocation failed or size overflow\n",stderr);exit(EXIT_FAILURE); }
void *malloc_assert(size_t size)
{
  void *p=malloc(size);
  if(!p) fail();
  return p;
}
void *calloc_assert(size_t n,size_t size)
{
  if(size && n>SIZE_MAX/size) fail();
  void *p=calloc(n,size);
  if(!p) fail();
  return p;
}
