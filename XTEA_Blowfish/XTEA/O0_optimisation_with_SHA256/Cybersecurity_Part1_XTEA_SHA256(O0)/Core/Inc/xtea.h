// xtea.h
#ifndef XTEA_H
#define XTEA_H

#include <stdint.h>

void xtea_enc(void *dest, const void *v, const void *k);
void xtea_dec(void *dest, const void *v, const void *k);

#endif // XTEA_H
