#include "vm.h"
#include "cpu.h"

static void load_program_mock(struct vm_data *vm) {
  const uint32_t mock_program[] = {0x007302b3, 0x0003ae13,     0x0062aeb3,
                                   0xfffff7b7, 0 * 0xff5ff2ef, 0};

  memcpy(vm->cpu->bus.dram.mem, mock_program, 5 * sizeof(uint32_t));
}

void load_program(struct vm_data *vm) { load_program_mock(vm); }

int vm_init(struct vm_data *vm, size_t memsize) {
  LOG_TRACE("vm_init(%p, %zu)", vm, memsize);

  struct rv_cpu *cpu = malloc(sizeof(struct rv_cpu));
  if (cpu == NULL) {
    LOG_ERROR("Could not allocate memory for rv_cpu");
    exit(ENOMEM);
  }

  int result = alloc_hugepage((void **)&cpu->bus.dram.mem, 4096, memsize);
  if (result) {
    LOG_ERROR("Could not allocate memory for DRAM: %d", result);
    exit(ENOMEM);
  }

  cpu->reg[rv_reg_zero] = 0;
  cpu->reg[rv_reg_sp] = RV_DRAM_BASE + memsize;
  cpu->pc = RV_DRAM_BASE;
  vm->cpu = cpu;

  return 0;
}

int vm_start(struct vm_data *vm) {
  LOG_TRACE("vm_start(%p)", vm);

  assert(vm != NULL);
  assert(vm->cpu != NULL);
  assert(vm->cpu->bus.dram.mem != NULL);

  load_program(vm);

  cpu_execute(vm->cpu);

  hexdump((const char *)vm->cpu->bus.dram.mem, 0, 0xA0);
  dump_core(vm->cpu);

  (void)vm;
  return 0;
}

void vm_shutdown(struct vm_data *vm) {
  LOG_TRACE("vm_shutdown(%p)", vm);

  free(vm->cpu->bus.dram.mem);
  free(vm->cpu);
}
