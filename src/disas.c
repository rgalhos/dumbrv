#include "disas.h"

const char *const rv_register_list[rv_reg_max] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

const rv_opcode_data rv_opcode_list[rv_opcode_max] = {
    // clang-format off
  {"illegal", rv_codec_illegal, ""},
    {"lui", rv_codec_u, rv_fmt_u},
    {"auipc", rv_codec_u, rv_fmt_u},
    {"jal", rv_codec_j, rv_fmt_j},
    {"jalr", rv_codec_i, rv_fmt_i},
    {"beq", rv_codec_b, rv_fmt_b},
    {"bne", rv_codec_b, rv_fmt_b},
    {"blt", rv_codec_b, rv_fmt_b},
    {"bge", rv_codec_b, rv_fmt_b},
    {"bltu", rv_codec_b, rv_fmt_b},
    {"bgeu", rv_codec_b, rv_fmt_b},
    {"lb", rv_codec_i, rv_fmt_i},
    {"lh", rv_codec_i, rv_fmt_i},
    {"lw", rv_codec_i, rv_fmt_i},
    {"lbu", rv_codec_i, rv_fmt_i},
    {"lhu", rv_codec_i, rv_fmt_i},
    {"sb", rv_codec_s, rv_fmt_s},
    {"sh", rv_codec_s, rv_fmt_s},
    {"sw", rv_codec_s, rv_fmt_s},
    {"addi", rv_codec_i, rv_fmt_i},
    {"slti", rv_codec_i, rv_fmt_i},
    {"sltiu", rv_codec_i, rv_fmt_i},
    {"xori", rv_codec_i, rv_fmt_i},
    {"ori", rv_codec_i, rv_fmt_i},
    {"andi", rv_codec_i, rv_fmt_i},
    {"slli", rv_codec_i, rv_fmt_i},
    {"srli", rv_codec_i, rv_fmt_i},
    {"srai", rv_codec_i, rv_fmt_i},
    {"add", rv_codec_r, rv_fmt_r},
    {"sub", rv_codec_r, rv_fmt_r},
    {"sll", rv_codec_r, rv_fmt_r},
    {"slt", rv_codec_r, rv_fmt_r},
    {"sltu", rv_codec_r, rv_fmt_r},
    {"xor", rv_codec_r, rv_fmt_r},
    {"srl", rv_codec_r, rv_fmt_r},
    {"sra", rv_codec_r, rv_fmt_r},
    {"or", rv_codec_r, rv_fmt_r},
    {"and", rv_codec_r, rv_fmt_r},
    {"fence", rv_codec_u, "todo"}, // <-- check later
    {"ecall", rv_codec_i, rv_fmt_i},
    {"ebreak", rv_codec_i, rv_fmt_i},
    {"lwu", rv_codec_u, "todo"},
    {"ld", rv_codec_u, "todo"},
    {"sd", rv_codec_u, "todo"},
    {"addiw", rv_codec_u, "todo"},
    {"slliw", rv_codec_u, "todo"},
    {"srliw", rv_codec_u, "todo"},
    {"sraiw", rv_codec_u, "todo"},
    {"addw", rv_codec_u, "todo"},
    {"subw", rv_codec_u, "todo"},
    {"sllw", rv_codec_u, "todo"},
    {"srlw", rv_codec_u, "todo"},
    {"sraw", rv_codec_u, "todo"},
    // RV64M
    // clang-format on
};

static inline uint32_t operand_opcode(const uint32_t inst) {
  return inst & 0x7F;
}

static inline uint32_t operand_rd(const uint32_t inst) {
  return (inst >> 7) & 0x1F;
}

static inline uint32_t operand_rs1(const uint32_t inst) {
  return (inst >> 15) & 0x1F;
}

static inline uint32_t operand_rs2(const uint32_t inst) {
  return (inst >> 20) & 0x1F;
}

static inline uint32_t operand_funct3(const uint32_t inst) {
  return (inst >> 12) & 0x7;
}

static inline uint32_t operand_funct7(const uint32_t inst) {
  return (inst >> 25) & 0x7F;
}

static inline uint32_t operand_iimm12(const uint32_t inst) {
  return inst >> 20;
}

static inline uint32_t operand_simm12(const uint32_t inst) {
  return (operand_iimm12(inst) << 5) | operand_funct3(inst);
}

static inline uint32_t operand_imm20(const uint32_t inst) {
  return ((inst >> 31) & 0x1) << 20 | ((inst >> 21) & 0x3FF) << 1 |
         ((inst >> 20) & 0x1) << 11 | ((inst >> 12) & 0xFF) << 12;
}

inline int64_t sext_i(uint32_t imm) {
#if RV_XLEN == 32
  return (int32_t)(imm << 20) >> 20;
#else
  return (int64_t)((uint64_t)imm << 52) >> 52;
#endif
}

// S-type: imm[11:5] + imm[4:0] combined
inline int64_t sext_s(uint32_t imm) {
#if RV_XLEN == 32
  return (int32_t)(imm << 20) >> 20;
#else
  return (int64_t)((uint64_t)imm << 52) >> 52;
#endif
}

// B-type: imm[12|10:5|4:1|11] combined, sign-extended
inline int64_t sext_b(uint32_t imm) {
#if RV_XLEN == 32
  return (int32_t)(imm << 19) >> 19;
#else
  return (int64_t)((uint64_t)imm << 51) >> 51;
#endif
}

// U-type: imm[31:12]
inline int64_t sext_u(uint32_t imm) { return (int64_t)imm; }

// J-type: imm[20|10:1|11|19:12], 12 bits
inline int64_t sext_j(uint32_t imm) {
#if RV_XLEN == 32
  return (int32_t)(imm << 12) >> 12;
#else
  return (int64_t)((uint64_t)imm << 44) >> 44;
#endif
}

void decode_inst(rv_decode *dec) {
  assert(dec != NULL);

  const uint32_t inst = dec->inst;
  const uint32_t opcode = operand_opcode(inst);
  dec->funct3 = operand_funct3(inst);
  dec->funct7 = operand_funct7(inst);

  rv_opcode op = rv_opcode_illegal;

  switch (opcode) {
    // clang-format off
  case 0b0110011 /* rv_opcode_add */: {
    switch (dec->funct3) {
      case 0x0: {
        switch (dec->funct7) {
          case 0x00: op = rv_opcode_add; break;
          case 0x20: op = rv_opcode_sub; break;
        }
      } break;
      case 0x1: op = rv_opcode_sll;  break;
      case 0x2: op = rv_opcode_slt;  break;
      case 0x3: op = rv_opcode_sltu; break;
      case 0x4: op = rv_opcode_xor;  break;
      case 0x5: op = rv_opcode_srl;  break;
      case 0x6: op = rv_opcode_or;   break;
      case 0x7: op = rv_opcode_and;  break;
    }
  } break;

  case 0b0010011 /* rv_opcode_addi */: {
    switch (dec->funct3) {
      case 0x0: op = rv_opcode_addi;  break;
      case 0x1: op = rv_opcode_slli;  break;
      case 0x2: op = rv_opcode_slti;  break;
      case 0x3: op = rv_opcode_sltiu; break;
      case 0x4: op = rv_opcode_xori;  break;
      case 0x5: {
        switch(dec->funct7) {
          case 0x00: op = rv_opcode_srli; break;
          case 0x20: op = rv_opcode_srai; break;
        }
      } break;
      case 0x6: op = rv_opcode_ori;   break;
      case 0x7: op = rv_opcode_andi;  break;
    }
  } break;

  case 0b0000011 /* rv_opcode_lb  */: {
    switch (dec->funct3) {
      case 0x0: op = rv_opcode_lb;  break;
      case 0x1: op = rv_opcode_lh;  break;
      case 0x2: op = rv_opcode_lw;  break;
      case 0x4: op = rv_opcode_lbu; break;
      case 0x5: op = rv_opcode_lhu; break;
    }
  } break;

  case 0b0100011 /* rv_opcode_sb */: {
    switch (dec->funct3) {
      case 0x0: op = rv_opcode_sb; break;
      case 0x1: op = rv_opcode_sh; break;
      case 0x2: op = rv_opcode_sw; break;
    }
  } break;

  case 0b1100011 /* rv_opcode_beq */: {
    switch (dec->funct3) {
      case 0x0: op = rv_opcode_beq;  break;
      case 0x1: op = rv_opcode_bne;  break;
      case 0x4: op = rv_opcode_blt;  break;
      case 0x5: op = rv_opcode_bge;  break;
      case 0x6: op = rv_opcode_bltu; break;
      case 0x7: op = rv_opcode_bgeu; break;
    }
  } break;

  case 0b1101111 /* rv_opcode_jal */: {
    op = rv_opcode_jal;
  } break;

  case 0b1100111 /* rv_opcode_jalr */: {
    op = rv_opcode_jalr;
  } break;

  case 0b0110111 /* rv_opcode_lui */: {
    op = rv_opcode_lui;
  } break;

  case 0b0010111 /* rv_opcode_auipc */: {
    op = rv_opcode_auipc;
  } break;

  case 0b1110011 /* rv_opcode_ecall */: {
    switch (dec->funct7) {
      case 0x0: op = rv_opcode_ecall;  break;
      case 0x1: op = rv_opcode_ebreak; break;
    }
  } break;
    // clang-format on
  }

  if (op == rv_opcode_illegal) {
    memset(dec, 0, sizeof(*dec));

    dec->op = rv_opcode_illegal;
    dec->codec = rv_codec_illegal;

    return;
  }

  dec->op = op;
  dec->codec = rv_opcode_list[dec->op].codec;
  dec->rd = (rv_reg)operand_rd(inst);
  dec->rs1 = (rv_reg)operand_rs1(inst);
  dec->rs2 = (rv_reg)operand_rs2(inst);

  switch ((rv_codec)dec->codec) {
  case rv_codec_i:
    dec->imm = operand_iimm12(inst);
    break;
  case rv_codec_s:
  case rv_codec_b:
    dec->imm = operand_simm12(inst);
    break;
  case rv_codec_u:
  case rv_codec_j:
    dec->imm = operand_imm20(inst);
    break;
  default:
    dec->imm = 0;
    break;
  }

  LOG_TRACE("decode_inst: op: %d, funct3: %d, funct7: %d, rd: %d, "
            "rs1: %d, rs2: %d, imm: %d, inst 0x%x",
            dec->op, dec->funct3, dec->funct7, dec->rd, dec->rs1, dec->rs2,
            dec->imm, dec->inst);
}

const char *reg_name(rv_reg r) { return rv_register_list[r]; }

size_t format_inst(const rv_decode dec, char *buf, const size_t maxlen) {
  char c, *fmt = (char *)rv_opcode_list[dec.op].fmt;
  size_t n = 0;

  while ((c = *fmt)) {
    if (c == 'O') {
      n += snprintf(buf + n, maxlen - n, "%s", rv_opcode_list[dec.op].name);
    } else if (c == 'd') {
      n += snprintf(buf + n, maxlen - n, "%s", rv_register_list[dec.rd]);
    } else if (c == '1') {
      n += snprintf(buf + n, maxlen - n, "%s", rv_register_list[dec.rs1]);
    } else if (c == '2') {
      n += snprintf(buf + n, maxlen - n, "%s", rv_register_list[dec.rs2]);
    } else if (c == 'i') {
      n += snprintf(buf + n, maxlen - n, "%d", dec.imm);
    } else if (c == 'x') {
      n += snprintf(buf + n, maxlen - n, "0x%X", dec.imm);
    } else if (c == 'j') {
      n += snprintf(buf + n, maxlen - n, "%ld", sext_j(dec.imm));
    } else if (n < maxlen - 1) {
      buf[n++] = c;
    } else {
      LOG_TRACE("format_inst: reached break (maxlen = %d, n = %d, c = '%c')",
                maxlen, n, c);
      break;
    }

    fmt++;
  }

  buf[n] = '\0';

  return n;
}
