#ifndef SHA256_H_
#define SHA256_H_
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t h[8]; uint64_t length; size_t used; unsigned char block[64]; } sha256;
void sha256_init(sha256 *);
void sha256_update(sha256 *,const void *,size_t);
void sha256_final(sha256 *,char out[65]);
#endif
