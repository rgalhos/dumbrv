#ifndef VM_H
#define VM_H

#include "cpu.h"
#include "disas.h"
#include "dram.h"
#include "log.h"
#include "memory.h"
#include "utils.h"
#include <assert.h>
#include <byteswap.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct vm_data {
  struct rv_cpu *cpu;
};

int vm_init(struct vm_data *, size_t);
int vm_start(struct vm_data *);
void vm_shutdown(struct vm_data *);

#endif // VM_H
