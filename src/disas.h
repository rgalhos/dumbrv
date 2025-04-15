#ifndef DISAS_H
#define DISAS_H

#include "cpu.h"
#include "log.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  rv_codec_illegal,
  rv_codec_r,
  rv_codec_i,
  rv_codec_s,
  rv_codec_b,
  rv_codec_j,
  rv_codec_u,
  rv_codec_max,
} rv_codec;

typedef enum {
  rv_opcode_illegal = 0,
  // opcode 0b0110111
  rv_opcode_lui,
  // opcode 0b0010111
  rv_opcode_auipc,
  // opcode 0b1101111
  rv_opcode_jal,
  // opcode 0b1100111
  rv_opcode_jalr,
  // opcode 0b1100011
  rv_opcode_beq,
  rv_opcode_bne,
  rv_opcode_blt,
  rv_opcode_bge,
  rv_opcode_bltu,
  rv_opcode_bgeu,
  // opcode 0b0000011
  rv_opcode_lb,
  rv_opcode_lh,
  rv_opcode_lw,
  rv_opcode_lbu,
  rv_opcode_lhu,
  // opcode 0b0100011
  rv_opcode_sb,
  rv_opcode_sh,
  rv_opcode_sw,
  // opcode 0b0010011
  rv_opcode_addi,
  rv_opcode_slti,
  rv_opcode_sltiu,
  rv_opcode_xori,
  rv_opcode_ori,
  rv_opcode_andi,
  rv_opcode_slli,
  rv_opcode_srli,
  rv_opcode_srai,
  // opcode 0b0110011
  rv_opcode_add,
  rv_opcode_sub,
  rv_opcode_sll,
  rv_opcode_slt,
  rv_opcode_sltu,
  rv_opcode_xor,
  rv_opcode_srl,
  rv_opcode_sra,
  rv_opcode_or,
  rv_opcode_and,
  // fence
  rv_opcode_fence,
  // opcode 0b1110011
  rv_opcode_ecall,
  rv_opcode_ebreak,
  // :::
  rv_opcode_lwu,
  rv_opcode_ld,
  rv_opcode_sd,
  rv_opcode_addiw,
  rv_opcode_slliw,
  rv_opcode_srliw,
  rv_opcode_sraiw,
  rv_opcode_addw,
  rv_opcode_subw,
  rv_opcode_sllw,
  rv_opcode_srlw,
  rv_opcode_sraw,

  rv_opcode_max
} rv_opcode;

typedef struct {
  const char *const name;
  const rv_codec codec;
  const char *const fmt;
} rv_opcode_data;

typedef struct {
  uint32_t inst;
  rv_codec codec;

  uint16_t op;
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  uint8_t rs3;
  uint32_t imm;
  uint8_t funct3;
  uint8_t funct7;

  // fence
  uint8_t fm;
  uint8_t pred;
  uint8_t succ;
} rv_decode;

// #define rv_fmt_r "O d,1,s"
#define rv_fmt_r "O d,1,2"
#define rv_fmt_i "O d,1,i"
#define rv_fmt_s "O i,1,2"
#define rv_fmt_b "O 1,2,x"
#define rv_fmt_j "O d,j"
#define rv_fmt_u "O d,j"

int64_t sext_i(uint32_t imm);
int64_t sext_s(uint32_t imm);
int64_t sext_b(uint32_t imm);
int64_t sext_u(uint32_t imm);
int64_t sext_j(uint32_t imm);
void decode_inst(rv_decode *);
size_t format_inst(const rv_decode, char *, const size_t);
const char *reg_name(rv_reg);

#endif // DISAS_H
