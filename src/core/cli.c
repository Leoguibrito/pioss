#include "cli.h"
#include "dts.h"
#include "mds.h"
uint64_t cli_slot_bytes(uint64_t size,uint64_t stripe,uint32_t width,uint32_t slot)
{
  uint64_t chunks=size/stripe;
  return (chunks/width)*stripe+(slot<chunks%width?stripe:0)+
         (slot==chunks%width?size%stripe:0);
}
uint32_t *cli_exec(uint32_t fid,uint64_t size,uint64_t stripe,uint32_t width)
{
  uint32_t *destinations=mds_open(fid);
  for(uint32_t i=0;i<width;++i)
    dts_store(destinations[i],cli_slot_bytes(size,stripe,width,i));
  return destinations;
}
