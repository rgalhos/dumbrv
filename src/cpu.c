#include "cpu.h"
#include "disas.h"
#include "dram.h"
#include "log.h"

static void exec_lui(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_lui;
  cpu->reg[d->rd] = sext_u(d->imm);
}

static void exec_auipc(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_auipc;
  // cpu->reg[d->rd] = cpu->pc + sext(d->imm << 12);
}

static void exec_jal(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_jal;
  cpu->reg[d->rd] = cpu->pc + (RV_ILEN / 8);
  cpu->pc = cpu->pc + sext_j(d->imm);
}

static void exec_jalr(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_jalr;
  uint64_t t = cpu->pc + (RV_ILEN / 8);
  cpu->pc = cpu->reg[d->rs1] + (sext_i(d->imm) & ~1);
  cpu->reg[d->rd] = t;
}

static void exec_bne(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_bne;
  if (cpu->reg[d->rs1] != cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + sext_b(d->imm);
  }
}

static void exec_addi(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_addi;
  cpu->reg[d->rd] = cpu->reg[d->rs1] + d->imm;
}

static void exec_slti(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_slti;
  cpu->reg[d->rd] = (signed)cpu->reg[d->rs1] < (signed)cpu->reg[d->imm];
}

static void exec_sltiu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_sltiu;
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] < (unsigned)cpu->reg[d->imm];
}

static void exec_add(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_addi;
  cpu->reg[d->rd] = cpu->reg[d->rs1] + cpu->reg[d->rs2];
}

static void exec_slt(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_slt;
  cpu->reg[d->rd] = (signed)cpu->reg[d->rs1] < (signed)cpu->reg[d->rs2];
}

uint32_t cpu_fetch(const struct rv_cpu *cpu) {
  return dram_read(cpu->bus.dram.mem, cpu->pc, 32);
}

int cpu_execute(struct rv_cpu *cpu) {
  uint32_t inst;
  rv_decode d;

  while (1) {
    inst = cpu_fetch(cpu);
    d.inst = inst;

    decode_inst(&d);

    if (d.op == rv_opcode_illegal) {
      LOG_ERROR("Illegal instruction: 0x%x", inst);
      return 1;
    }

    char disas[64];
    format_inst(d, disas, 63);
    LOG_TRACE("%s", disas);

    switch ((rv_opcode)d.op) {
      // clang-format off
    case rv_opcode_lui:   exec_lui(cpu, &d);   break;
    case rv_opcode_auipc: exec_auipc(cpu, &d); break;
    case rv_opcode_jal:   exec_jal(cpu, &d);   break;
    case rv_opcode_jalr:  exec_jalr(cpu, &d);  break;
    case rv_opcode_bne:   exec_bne(cpu, &d);   break;
    case rv_opcode_addi:  exec_addi(cpu, &d);  break;
    case rv_opcode_slti:  exec_slti(cpu, &d);  break;
    case rv_opcode_sltiu: exec_sltiu(cpu, &d); break;
    case rv_opcode_add:   exec_add(cpu, &d);   break;
    case rv_opcode_slt:   exec_slt(cpu, &d);   break;
      // clang-format on
    default: {
      LOG_WARN("Not implemented");
      return 1;
    }
    }

    cpu->pc += RV_ILEN / 8;
  }

  return 0;
}

void dump_core(const struct rv_cpu *cpu) {
#define FREG "%4s [0x%.8Xh]"
#define SREG(r) reg_name(r), cpu->reg[r]

  for (rv_reg r = rv_reg_zero; r < rv_reg_max; r += 4) {
    LOG_INFO(FREG FREG FREG FREG, SREG(r), SREG(r + 1), SREG(r + 2),
             SREG(r + 3));
  }

  LOG_INFO(FREG, "pc", cpu->pc);

#undef FREG
#undef SREG
}
