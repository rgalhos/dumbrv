#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/logging.h>
#include <stdint.h>
#include <string.h>

#include "../src/cpu.h"
#include "../src/vm.h"

#define VM_TEST_MEM_SIZE 1024

#define ARR(...) __VA_ARGS__
#define SET_VM_MEM(V, S, M)                                                    \
  do {                                                                         \
    const uint32_t inst[] = M;                                                 \
    memcpy((V).cpu->bus.dram.mem, inst, S * sizeof(uint32_t));                 \
  } while (0)

#define VM_INIT(V)                                                             \
  struct vm_data V;                                                            \
  vm_init(&V, VM_TEST_MEM_SIZE)

#define VM_SHUTDOWN(V) vm_shutdown(&V)

Test(mv, init_and_shutdown) {
  VM_INIT(vm);

  cr_assert_not_null(vm.cpu);
  cr_assert_not_null(vm.cpu->bus.dram.mem);
  cr_assert_eq(vm.cpu->bus.dram.size, VM_TEST_MEM_SIZE);
  cr_assert_eq(vm.cpu->xlen, RV_ARCH_RV64);

  VM_SHUTDOWN(vm);
}

Test(cpu, exec_lui) {
  VM_INIT(vm);

  // 0x004d2737 = lui a4, 1234
  // 0xfffff7b7 = lui a5, 1048575 (-1)
  // 0x00000837 = lui a6, 1048576 (0) (overflow = 0)
  // 0x000018b7 = lui a7, 1
  SET_VM_MEM(vm, 5, ARR({0x004d2737, 0x03039737, 0x1e240737, 0x2d687737, 0}));

  cpu_execute(vm.cpu);

  cr_expect_eq((signed)vm.cpu->reg[rv_reg_a4], 1234);
  cr_expect_eq((signed)vm.cpu->reg[rv_reg_a5], 12345);
  cr_expect_eq((signed)vm.cpu->reg[rv_reg_a6], 123456);
  cr_expect_eq((signed)vm.cpu->reg[rv_reg_a7], 1234567);

  cr_log_warn("aaaaaaaaaaa %lld %lld %lld %lld",
              (signed long long)vm.cpu->reg[rv_reg_a4],
              (signed long long)vm.cpu->reg[rv_reg_a5],
              (signed long long)vm.cpu->reg[rv_reg_a6],
              (signed long long)vm.cpu->reg[rv_reg_a7]);

  VM_SHUTDOWN(vm);
}

Test(cpu, exec_auipc) {
  // auipc
}

Test(cpu, exec_jal) {
  VM_INIT(vm);

  // jal a4, 768
  SET_VM_MEM(vm, 2, ARR({0x3000076f, 0}));

  cpu_execute(vm.cpu);

  cr_expect_eq(vm.cpu->pc, 768 + 4);
  cr_expect_eq(vm.cpu->reg[rv_reg_a4], 8);

  cr_log_warn("aaaaaaaaaaa %llu %llu",
              (unsigned long long)vm.cpu->reg[rv_reg_a4],
              (unsigned long long)vm.cpu->pc);

  VM_SHUTDOWN(vm);
}

Test(cpu, exec_add) {
  VM_INIT(vm);

  vm.cpu->reg[rv_reg_a2] = (uint64_t)-1;
  vm.cpu->reg[rv_reg_a3] = 1024;
  vm.cpu->reg[rv_reg_a6] = 12345;
  vm.cpu->reg[rv_reg_a7] = 12345;

  // 0x00d605b3 = add a1, a2, a3
  // 0x011807b3 = add a5, a6, a7
  SET_VM_MEM(vm, 3, ARR({0x00d605b3, 0x011807b3, 0x0}));

  cpu_execute(vm.cpu);

  cr_expect_eq(vm.cpu->reg[rv_reg_a1], ((uint64_t)-1) + 1024);
  cr_expect_eq(vm.cpu->reg[rv_reg_a5], 24690);

  VM_SHUTDOWN(vm);
}
