// asm.h
//
#ifndef ASM_h
#define ASM_h

enum Asm {
  ADD,
  ADDI,
  SUB,
  MUL,
  DIV,
  MOD,
  SLT,
  AND,
  ANDI,
  OR,
  ORI,
  NOR,
  XOR,
  XORI,
  SRAI,
  SRLI,
  SLLI,
  SRA,
  SRL,
  SLL,
  FADD,
  FSUB,
  FMUL,
  FDIV,
  SQRT,
  LW,
  LB,
  SW,
  SB,
  MOV,
  BC,
  BEQ,
  BNE,
  J,
  JR,
  JAL,
  JALR,
  NOP,
  OUT
};

#endif
