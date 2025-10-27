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

  SET_VM_MEM(vm, 6,
             ARR({
                 0x00000637, // lui a2, 0
                 0xfffff6b7, // lui a3, 0xfffff
                 0x004d2737, // lui a4, 0x004d2 (1234)
                 0x030397b7, // lui a5, 0x03039 (12345)
                 0x1e240837, // lui a6, 0x1e240 (123456)
                 0xc0ffe8b7, // lui a7, 0xc0ffe
                 0,
             }));

  cpu_execute(vm.cpu);

  cr_expect_eq(vm.cpu->reg[rv_reg_a2], 0);
  cr_expect_eq(vm.cpu->reg[rv_reg_a3], 0xfffff);
  cr_expect_eq(vm.cpu->reg[rv_reg_a4], 1234);
  cr_expect_eq(vm.cpu->reg[rv_reg_a5], 12345);
  cr_expect_eq(vm.cpu->reg[rv_reg_a6], 123456);
  cr_expect_eq(vm.cpu->reg[rv_reg_a7], 0xc0ffe);

  VM_SHUTDOWN(vm);
}

Test(cpu, exec_auipc) {
  VM_INIT(vm);

  SET_VM_MEM(vm, 4,
             ARR({
                 0x00000597, // auipc a1, 0
                 0x00400617, // auipc a2, 1024
                 0xffffc697, // auipc a3, -4
                 0x0,        // end
             }));

  cpu_execute(vm.cpu);

  cr_assert_eq(vm.cpu->reg[rv_reg_a1], RV_DRAM_BASE + 0);
  cr_assert_eq(vm.cpu->reg[rv_reg_a2], RV_DRAM_BASE + 4 * 1 + 1024);
  cr_assert_eq(vm.cpu->reg[rv_reg_a3], RV_DRAM_BASE + 4 * 2 - 4);

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

Test(cpu, exec_jal) {
  VM_INIT(vm);

  SET_VM_MEM(vm, 8,
             ARR({
                 0x004000ef, // jal ra, +4
                 0x02a00793, // addi a5, zero, 42 // jump this one
                 0x06300813, // addi a6, zero, 99
                 0x0080076f, // jal a4, +8
                 0x04500593, // addi a1, zero, 69   // jump this
                 0x53900613, // addi a2, zero, 1337 // jump this
                 0xabc00693, // addi a3, zero, 0xABC
                 0x00000000  // end
             }));

  vm.cpu->reg[rv_reg_a1] = 0xC0FFE;
  vm.cpu->reg[rv_reg_a2] = 0xC0FFE;
  vm.cpu->reg[rv_reg_a5] = 0xC0FFE;

  cpu_execute(vm.cpu);

  cr_expect_eq(vm.cpu->reg[rv_reg_ra], RV_DRAM_BASE + 4);
  cr_expect_eq(vm.cpu->reg[rv_reg_a5], 0xC0FFE);
  cr_expect_eq(vm.cpu->reg[rv_reg_a6], 99);

  cr_expect_eq(vm.cpu->reg[rv_reg_a4], RV_DRAM_BASE + 4 * 4);
  cr_expect_eq(vm.cpu->reg[rv_reg_a1], 0xC0FFE);
  cr_expect_eq(vm.cpu->reg[rv_reg_a2], 0xC0FFE);
  cr_expect_eq(vm.cpu->reg[rv_reg_a3], 0xABC);

  VM_SHUTDOWN(vm);
}

Test(cpu, exec_jalr) {
  VM_INIT(vm);

  // jalr ra, 8(ra)
  // addi t0, zero, 42     // should be skipped
  // addi t1, zero, 99
  // end
  // addi t2, zero, 666
  // auipc a7, zero
  // jalr a7, -8(ra)

  SET_VM_MEM(vm, 8,
             ARR({
                 0x004080e7, // jalr ra, ra, +4       (jump to ra + 4)
                 0x02a00793, // addi a5, zero, 42     // should be skipped
                 0x06300813, // addi a6, zero, 99
                 0x00408767, // jalr a4, ra, +4       (jump again)
                 0x04500593, // addi a1, zero, 69     // should be skipped
                 0x53900613, // addi a2, zero, 1337   // should be skipped
                 0xabc00693, // addi a3, zero, 0xABC
                 0x00000000  // end
             }));

  // Initialize registers
  vm.cpu->reg[rv_reg_ra] = RV_DRAM_BASE;
  vm.cpu->reg[rv_reg_a1] = 0xC0FFE;
  vm.cpu->reg[rv_reg_a2] = 0xC0FFE;
  vm.cpu->reg[rv_reg_a5] = 0xC0FFE;

  cpu_execute(vm.cpu);

  // First jalr: rd=ra, jumps to ra+4
  cr_expect_eq(vm.cpu->reg[rv_reg_ra], RV_DRAM_BASE + 4);
  // The skipped instruction didn’t run
  cr_expect_eq(vm.cpu->reg[rv_reg_a5], 0xC0FFE);
  // The next executed instruction set a6=99
  cr_expect_eq(vm.cpu->reg[rv_reg_a6], 99);

  // Second jalr: rd=a4, jumps to (ra + 4)
  cr_expect_eq(vm.cpu->reg[rv_reg_a4], RV_DRAM_BASE + 8);
  cr_expect_eq(vm.cpu->reg[rv_reg_a1], 0xC0FFE);
  cr_expect_eq(vm.cpu->reg[rv_reg_a2], 0xC0FFE);
  cr_expect_eq(vm.cpu->reg[rv_reg_a3], 0xABC);

  VM_SHUTDOWN(vm);
}
