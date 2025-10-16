#ifndef DRAM_H
#define DRAM_H

#include <stdint.h>
#include <string.h>

#define RV_DRAM_BASE 0x80000000

struct rv_dram {
  uint8_t *mem;
  uint64_t size;
};

void dram_write(uint8_t *, uint64_t, size_t, uint8_t);
uint64_t dram_read(const uint8_t *, size_t, uint8_t);

#endif // DRAM_H
