#ifndef MDS_H_
#define MDS_H_
#include "param.h"
void mds_init(param);
uint32_t *mds_open(uint32_t);
void mds_counts(uint64_t *,uint32_t *);
void mds_clean(void);
#endif
