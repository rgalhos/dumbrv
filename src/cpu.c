#include "cpu.h"
#include "disas.h"
#include "dram.h"
#include "log.h"

static inline void exec_lui(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_lui;
  cpu->reg[d->rd] = sext_u(d->imm);
}

static inline void exec_auipc(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_auipc;
  cpu->reg[d->rd] = cpu->pc + (signed)sext_u(d->imm);
}

static inline void exec_jal(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_jal;
  cpu->reg[d->rd] = cpu->pc + (RV_ILEN / 8);
  cpu->pc = cpu->pc + sext_j(d->imm);
}

static inline void exec_jalr(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_jalr;
  uint64_t t = cpu->pc + (RV_ILEN / 8);
  cpu->pc = cpu->reg[d->rs1] + (sext_i(d->imm) & ~1);
  cpu->reg[d->rd] = t;
}

static inline void exec_beq(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_beq;
  if ((signed)cpu->reg[d->rs1] == (signed)cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_bne(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_bne;
  if ((signed)cpu->reg[d->rs1] != (signed)cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_blt(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_blt;
  if ((signed)cpu->reg[d->rs1] < (signed)cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_bge(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_bge;
  if ((signed)cpu->reg[d->rs1] >= (signed)cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_bltu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_bltu;
  if (cpu->reg[d->rs1] < cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_bgeu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_bgeu;
  if (cpu->reg[d->rs1] >= cpu->reg[d->rs2]) {
    cpu->pc = cpu->pc + (signed)sext_b(d->imm);
  }
}

static inline void exec_lb(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_lb;
  cpu->reg[d->rd] = sext_b(
      dram_read(cpu->bus.dram.mem, cpu->reg[d->rs1] + (signed)d->imm, 8));
}

static inline void exec_lh(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_lh;
  cpu->reg[d->rd] = sext_b(
      dram_read(cpu->bus.dram.mem, cpu->reg[d->rs1] + (signed)d->imm, 16));
}

static inline void exec_lbu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_lbu;
  cpu->reg[d->rd] = dram_read(cpu->bus.dram.mem, cpu->reg[d->rs1] + d->imm, 8);
}

static inline void exec_lhu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_lhu;
  cpu->reg[d->rd] = dram_read(cpu->bus.dram.mem, cpu->reg[d->rs1] + d->imm, 16);
}

static inline void exec_sb(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_sb;
  dram_write(cpu->bus.dram.mem, cpu->reg[d->rs1] + d->imm, cpu->reg[d->rs2], 8);
}

static inline void exec_sh(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_sh;
  dram_write(cpu->bus.dram.mem, cpu->reg[d->rs1] + d->imm, cpu->reg[d->rs2],
             16);
}

static inline void exec_sw(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_sw;
  dram_write(cpu->bus.dram.mem, cpu->reg[d->rs1] + d->imm, cpu->reg[d->rs2],
             32);
}

static inline void exec_addi(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_addi;
  cpu->reg[d->rd] = cpu->reg[d->rs1] + d->imm;
}

static inline void exec_slti(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_slti;
  cpu->reg[d->rd] = (signed)cpu->reg[d->rs1] < (signed)cpu->reg[d->imm];
}

static inline void exec_sltiu(struct rv_cpu *cpu, const rv_decode *d) {
  (void)rv_opcode_sltiu;
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] < (unsigned)cpu->reg[d->imm];
}

static inline void exec_xori(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_xori;
  cpu->reg[d->rd] = cpu->reg[d->rs1] ^ (signed)d->imm;
}

static inline void exec_ori(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_ori;
  cpu->reg[d->rd] = cpu->reg[d->rs1] | (signed)d->imm;
}

static inline void exec_andi(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_andi;
  cpu->reg[d->rd] = cpu->reg[d->rs1] & (signed)d->imm;
}

static inline void exec_slli(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  // @todo shamt é diferente pra rv32 e rv64
  (void)rv_opcode_slli;
  cpu->reg[d->rd] = cpu->reg[d->rs1] << d->imm;
}

static inline void exec_srli(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  // @todo shamt é diferente pra rv32 e rv64
  (void)rv_opcode_slli;
  cpu->reg[d->rd] = cpu->reg[d->rs1] >> d->imm;
}

static inline void exec_srai(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  // @todo shamt é diferente pra rv32 e rv64
  (void)rv_opcode_srai;
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] >> (unsigned)cpu->reg[d->imm];
}

static inline void exec_add(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_add;
  cpu->reg[d->rd] = cpu->reg[d->rs1] + cpu->reg[d->rs2];
}

static inline void exec_sub(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_sub;
  cpu->reg[d->rd] = cpu->reg[d->rs1] - cpu->reg[d->rs2];
}

static inline void exec_sll(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_sll;
  // @todo muda pra rv64?
  // https://riscv-software-src.github.io/riscv-unified-db/manual/html/isa/isa_20240411/insts/sll.html
  cpu->reg[d->rd] = cpu->reg[d->rs1] << cpu->reg[d->rs2];
}

static inline void exec_slt(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_slt;
  cpu->reg[d->rd] = (signed)cpu->reg[d->rs1] < (signed)cpu->reg[d->rs2];
}

static inline void exec_sltu(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_sltu;
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] < (unsigned)cpu->reg[d->rs2];
}

static inline void exec_xor(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_xor;
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] ^ (unsigned)cpu->reg[d->rs2];
}

static inline void exec_srl(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_srl;
  // @todo muda pra rv64?
  // https://riscv-software-src.github.io/riscv-unified-db/manual/html/isa/isa_20240411/insts/srl.html
  cpu->reg[d->rd] = (signed)cpu->reg[d->rs1] >> (signed)cpu->reg[d->rs2];
}

static inline void exec_sra(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_sra;
  // @todo muda pra rv64?
  // https://riscv-software-src.github.io/riscv-unified-db/manual/html/isa/isa_20240411/insts/sra.html
  cpu->reg[d->rd] = (unsigned)cpu->reg[d->rs1] >> (unsigned)cpu->reg[d->rs2];
}

static inline void exec_or(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_or;
  cpu->reg[d->rd] = cpu->reg[d->rs1] | cpu->reg[d->rs2];
}

static inline void exec_and(struct rv_cpu *cpu, const rv_decode *d) {
  // Zkt ext?
  (void)rv_opcode_and;
  cpu->reg[d->rd] = cpu->reg[d->rs1] & cpu->reg[d->rs2];
}

uint32_t cpu_fetch(const struct rv_cpu *cpu) {
  return dram_read(cpu->bus.dram.mem, cpu->pc, RV_ILEN);
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
      // RV32I
    case rv_opcode_lui:   exec_lui(cpu, &d);   break;
    case rv_opcode_auipc: exec_auipc(cpu, &d); break;
    case rv_opcode_jal:   exec_jal(cpu, &d);   break;
    case rv_opcode_jalr:  exec_jalr(cpu, &d);  break;
    case rv_opcode_beq:   exec_beq(cpu, &d);   break;
    case rv_opcode_bne:   exec_bne(cpu, &d);   break;
    case rv_opcode_blt:   exec_blt(cpu, &d);   break;
    case rv_opcode_bge:   exec_bge(cpu, &d);   break;
    case rv_opcode_bltu:  exec_bltu(cpu, &d);  break;
    case rv_opcode_bgeu:  exec_bgeu(cpu, &d);  break;
    case rv_opcode_lb:    exec_lb(cpu, &d);    break;
    case rv_opcode_lh:    exec_lh(cpu, &d);    break;
    case rv_opcode_lbu:   exec_lbu(cpu, &d);   break;
    case rv_opcode_lhu:   exec_lhu(cpu, &d);   break;
    case rv_opcode_sb:    exec_sb(cpu, &d);    break;
    case rv_opcode_sh:    exec_sh(cpu, &d);    break;
    case rv_opcode_sw:    exec_sw(cpu, &d);    break;
    case rv_opcode_addi:  exec_addi(cpu, &d);  break;
    case rv_opcode_slti:  exec_slti(cpu, &d);  break;
    case rv_opcode_sltiu: exec_sltiu(cpu, &d); break;
    case rv_opcode_xori:  exec_xori(cpu, &d);  break;
    case rv_opcode_ori:   exec_ori(cpu, &d);   break;
    case rv_opcode_andi:  exec_andi(cpu, &d);  break;
    case rv_opcode_slli:  exec_slli(cpu, &d);  break;
    case rv_opcode_srli:  exec_srli(cpu, &d);  break;
    case rv_opcode_srai:  exec_srai(cpu, &d);  break;
    case rv_opcode_add:   exec_add(cpu, &d);   break;
    case rv_opcode_sub:   exec_sub(cpu, &d);   break;
    case rv_opcode_slt:   exec_slt(cpu, &d);   break;
    case rv_opcode_sltu:  exec_sltu(cpu, &d);  break;
    case rv_opcode_xor:   exec_xor(cpu, &d);   break;
    case rv_opcode_srl:   exec_srl(cpu, &d);   break;
    case rv_opcode_sra:   exec_sra(cpu, &d);   break;
    case rv_opcode_or:    exec_or(cpu, &d);    break;
    case rv_opcode_and:   exec_and(cpu, &d);   break;
      // RV64I
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
