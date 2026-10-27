#include "param.h"
#include "pioss.h"
#include "screen.h"
#include <stdlib.h>
int main(int argc,char **argv)
{
  param p;
  if(param_parse(argc,argv,&p) || param_validate(p)) return EXIT_FAILURE;
  if(!p.is_quiet) {
    char *description=param_tostr(p);
    if(!description) return EXIT_FAILURE;
    screen("PIOSS Lite");screen(description);free(description);
  }
  return pioss_exec(p)?EXIT_FAILURE:EXIT_SUCCESS;
}
