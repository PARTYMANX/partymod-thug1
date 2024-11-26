#ifndef _BPS_H_
#define _BPS_H_

#include <stdint.h>

int applyPatch(uint8_t *patch, size_t patchLen, uint8_t *input, size_t inputLen, uint8_t **output, size_t *outputLen);

#endif