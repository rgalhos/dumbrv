#include "dram.h"
#include "cpu.h"

void dram_write(uint8_t *dram, const size_t addr, RV_UINT_XLEN val,
                uint8_t size) {
  memcpy(dram + addr - RV_DRAM_BASE, &val, size / 8);

  switch (size) {
#if RV_XLEN == 64
  case 64:
    dram[addr - RV_DRAM_BASE + 7] = (uint8_t)((val >> 56) & 0xFF);
    dram[addr - RV_DRAM_BASE + 6] = (uint8_t)((val >> 48) & 0xFF);
    dram[addr - RV_DRAM_BASE + 5] = (uint8_t)((val >> 40) & 0xFF);
    dram[addr - RV_DRAM_BASE + 4] = (uint8_t)((val >> 32) & 0xFF);
    __attribute__((fallthrough));
#endif
  case 32:
    dram[addr - RV_DRAM_BASE + 3] = (uint8_t)((val >> 24) & 0xFF);
    dram[addr - RV_DRAM_BASE + 2] = (uint8_t)((val >> 16) & 0xFF);
    __attribute__((fallthrough));
  case 16:
    dram[addr - RV_DRAM_BASE + 1] = (uint8_t)((val >> 8) & 0xFF);
  }

  dram[addr - RV_DRAM_BASE] = (uint8_t)(val & 0xFF);
}

uint64_t dram_read(const uint8_t *dram, const size_t addr, uint8_t size) {
  uint64_t v = (uint64_t)dram[addr - RV_DRAM_BASE];

  switch (size) {
#if RV_XLEN == 64
  case 64:
    v |= ((uint64_t)dram[addr - RV_DRAM_BASE + 7] << 56) |
         ((uint64_t)dram[addr - RV_DRAM_BASE + 6] << 48) |
         ((uint64_t)dram[addr - RV_DRAM_BASE + 5] << 40) |
         ((uint64_t)dram[addr - RV_DRAM_BASE + 4] << 32);
    __attribute__((fallthrough));
#endif
  case 32:
    v |= ((uint64_t)dram[addr - RV_DRAM_BASE + 3] << 24) |
         ((uint64_t)dram[addr - RV_DRAM_BASE + 2] << 16);
    __attribute__((fallthrough));
  case 16:
    v |= ((uint64_t)dram[addr - RV_DRAM_BASE + 1] << 8);
  }

  return v;
}
