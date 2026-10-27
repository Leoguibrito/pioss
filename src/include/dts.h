#ifndef DTS_H_
#define DTS_H_
#include <stdint.h>
typedef struct { uint32_t num_dts;uint64_t *bytes_stored; } dts_results;
void dts_init(uint32_t);
void dts_store(uint32_t,uint64_t);
uint64_t dts_load(uint32_t);
void dts_clean(void);
dts_results *dts_get_results(void);
#endif
