#ifndef CPU_H
#define CPU_H

#include "dram.h"
#include <stdint.h>

#define RV_PAGE_SIZE 4096
#define RV_XLEN 64
#define RV_ILEN 32
#define RV_IALIGN 32

#if RV_XLEN == 32
#define RV_UINT_XLEN uint32_t
#define RV_INT_XLEN int32_t
#define RV_ADDRESS_SPACE ((uint32_t)-1)
#else
#define RV_UINT_XLEN uint64_t
#define RV_INT_XLEN int64_t
#define RV_ADDRESS_SPACE ((uint64_t)-1)
#endif

typedef enum {
  rv_reg_zero,
  rv_reg_ra,
  rv_reg_sp,
  rv_reg_gp,
  rv_reg_tp,
  rv_reg_t0,
  rv_reg_t1,
  rv_reg_t2,
  rv_reg_s0,
  rv_reg_s1,
  rv_reg_a0,
  rv_reg_a1,
  rv_reg_a2,
  rv_reg_a3,
  rv_reg_a4,
  rv_reg_a5,
  rv_reg_a6,
  rv_reg_a7,
  rv_reg_s2,
  rv_reg_s3,
  rv_reg_s4,
  rv_reg_s5,
  rv_reg_s6,
  rv_reg_s7,
  rv_reg_s8,
  rv_reg_s9,
  rv_reg_s10,
  rv_reg_s11,
  rv_reg_t3,
  rv_reg_t4,
  rv_reg_t5,
  rv_reg_t6,
  rv_reg_max,
} rv_reg;

struct rv_bus {
  struct rv_dram dram;
};

struct rv_cpu {
  RV_UINT_XLEN reg[rv_reg_max];
  RV_UINT_XLEN pc;

  struct rv_bus bus;
};

void cpu_init(struct rv_cpu *);
uint32_t cpu_fetch(const struct rv_cpu *);
int cpu_execute(struct rv_cpu *);
void dump_core(const struct rv_cpu *);

#endif // CPU_H
