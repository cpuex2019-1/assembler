// controller.cpp:
// execute instructions

#include "controller.h"
#include "asm.h"
#include "global.h"
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

void print_binary(unsigned int v) {
  unsigned int mask = 1 << 31;
  do {
    if (0x2108420 & mask) { // 00000010000100001000010000100000
      putchar(' ');
    }
    putchar(mask & v ? '1' : '0');
  } while (mask >>= 1);
}

controller::controller(const char *fname, loader *l, Log *l_level) {
  ld = l;
  log_level = l_level;
  program_num = 0;
  program_counter = 0;

  string filename = fname;
  filename.pop_back(); // 最後のsを削除
  filename = filename + "coe";
  outputfile = fopen(filename.c_str(), "w");
  if (outputfile == NULL) { // オープンに失敗した場合
    printf("cannot open file\n");
    exit(1);
  }
  fprintf(outputfile,
          "memory_initialization_radix=16;\nmemory_initialization_vector=\n");
}

void controller::assemble() {

  while (program_num < ld->end_line_num) {
    vector<int> line_vec = ld->get_program_by_line_num(program_num);
    if (*log_level >= DEBUG) { // raw_programを出力
      printf("\n");
      string one_raw_program = ld->get_raw_program_by_line_num(program_num);
      printf("DEBUG\t%d:\t%s\n", program_num, one_raw_program.c_str());
    }
    if (*log_level >= TRACE) { // programを出力
      printf("TRACE\t");
      for (auto itr_str = line_vec.begin(); itr_str != line_vec.end();
           ++itr_str) {
        printf("%d\t", *itr_str);
      }
      printf("\n");
    }

    exec_code(line_vec);

    program_num++;
    program_counter += 4;

    if (program_num >= ld->end_line_num) {
      fprintf(outputfile, ";\n");
    } else {
      fprintf(outputfile, ",\n");
    }
  }
  fclose(outputfile);
}

void controller::exec_code(vector<int> line_vec) {
  /*
    unsigned int op = 0xFE000000;          //上位6bit(<< 26)
    unsigned int rd = 0x03E00000;          // 5bit(<< 21)
    unsigned int rs = 0x001F0000;          // 5bit(<< 16)
    unsigned int rt = 0x0000F800;          // 5bit(<< 11)
    unsigned int shamt = 0x000007C0;       // 5bit(<< 6)
    unsigned int funct = 0x0000003F;       // 6bit(<< 0)
    unsigned int five = 0x0000001F;        // 5bit
    unsigned int six = 0x0000003F;         // 6bit
    unsigned int addr_or_imm = 0x0000FFFF; // 16bit
    unsigned int address = 0x03FFFFFF;     // 26bit
    */

  auto iter = line_vec.begin();
  int opecode = *iter;
  iter++;

  if (opecode == ADD) { // ADD rd <- rs + rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = 0x0;
    unsigned int funct_bit = 0x20;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == ADDI) { // ADDI rd <- rs + immediate
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int immediate = *iter;

    unsigned int op_bit = 0x8 << 26;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int immediate_bit = immediate;

    unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SUB) { // SUB rd <- rs - rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = 0x0;
    unsigned int funct_bit = 0x22;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == MUL) { // MUL rd <- rs * rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x2 << 6);
    unsigned int funct_bit = 0x18;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == DIV) { // DIV rd <- rs / rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x2 << 6);
    unsigned int funct_bit = 0x1A;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == MOD) { // MOD rd <- rs % rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x3 << 6);
    unsigned int funct_bit = 0x1A;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SLT) { // SLT Rd = if Rs < Rt then 1 else 0
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x0 << 6);
    unsigned int funct_bit = 0x2A;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == AND) { // AND rd <- rs & rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x0 << 6);
    unsigned int funct_bit = 0x24;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == ANDI) { // ANDI rd <- rs & immediate
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int immediate = *iter;

    unsigned int op_bit = 0xC << 26;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int immediate_bit = immediate;

    unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == OR) { // OR rd <- rs | rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x0 << 6);
    unsigned int funct_bit = 0x25;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == ORI) { // ORI rd <- rs & immediate
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int immediate = *iter;

    unsigned int op_bit = 0xD << 26;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int immediate_bit = immediate;

    unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == NOR) { // NOR rd <- ~(rs | rt)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x0 << 6);
    unsigned int funct_bit = 0x27;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == XOR) { // XOR rd <- rs ^ rt
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = (0x0 << 6);
    unsigned int funct_bit = 0x26;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == XORI) { // XORI rd <- rs & immediate
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int immediate = *iter;

    unsigned int op_bit = 0xE << 26;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int immediate_bit = immediate;

    unsigned int code = op_bit | rd_bit | rs_bit | immediate_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SRAI) { // SRAI rd <- rs >> sb (arithmetic)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int sb = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = 0x0;
    unsigned int shamt_bit = ((unsigned int)sb << 6);
    unsigned int funct_bit = 0x3;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SRLI) { // SRLI rd <- rs >> sb (logical)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int sb = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = 0x0;
    unsigned int shamt_bit = ((unsigned int)sb << 6);
    unsigned int funct_bit = 0x2;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SLLI) { // SLLI rd <- rs << sb (logical)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int sb = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = 0x0;
    unsigned int shamt_bit = ((unsigned int)sb << 6);
    unsigned int funct_bit = 0x0;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SRA) { // SRA rd <- rs >> rt (arithmetic)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = 0x0;
    unsigned int funct_bit = 0x7;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SRL) { // SRL rd <- rs >> rt (logical)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = 0x0;
    unsigned int funct_bit = 0x6;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SLL) { // SLL rd <- rs << rt (logical)
    int rd = *iter;
    iter++;
    int rs = *iter;
    iter++;
    int rt = *iter;

    unsigned int op_bit = 0x0;
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int rs_bit = ((unsigned int)rs << 16);
    unsigned int rt_bit = ((unsigned int)rt << 11);
    unsigned int shamt_bit = 0x0;
    unsigned int funct_bit = 0x4;

    unsigned int code =
        op_bit | rd_bit | rs_bit | rt_bit | shamt_bit | funct_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == LW) { // LW rd, offset(base)
    int rd = *iter;
    iter++;
    int base = *iter;
    iter++;
    int offset = *iter;

    unsigned int op_bit = (0x23 << 26);
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int base_bit = ((unsigned int)base << 16);
    unsigned int offset_bit = (unsigned int)offset;

    unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == LB) { // LB rd, offset(base)
    int rd = *iter;
    iter++;
    int base = *iter;
    iter++;
    int offset = *iter;

    unsigned int op_bit = (0x20 << 26);
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int base_bit = ((unsigned int)base << 16);
    unsigned int offset_bit = (unsigned int)offset;

    unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SW) {
    int rd = *iter;
    iter++;
    int base = *iter;
    iter++;
    int offset = *iter;

    unsigned int op_bit = (0x2B << 26);
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int base_bit = ((unsigned int)base << 16);
    unsigned int offset_bit = (unsigned int)offset;

    unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == SB) { // SB rd, offset(base)
    int rd = *iter;
    iter++;
    int base = *iter;
    iter++;
    int offset = *iter;

    unsigned int op_bit = (0x28 << 26);
    unsigned int rd_bit = ((unsigned int)rd << 21);
    unsigned int base_bit = ((unsigned int)base << 16);
    unsigned int offset_bit = (unsigned int)offset;

    unsigned int code = op_bit | rd_bit | base_bit | offset_bit;

    if (*log_level >= DEBUG) {
      printf("hex(16):%08x\tbinary:", code);
      print_binary(code);
      printf("\n");
    }

    fprintf(outputfile, "%08x", code);

  } else if (opecode == MOV) { // MOV rd <- rs
    int rd = *iter;
    iter++;
    int rs = *iter;

  } else if (opecode == BC) { // BC label(pc+offset<<2)
    program_num = *iter;

  } else if (opecode == BEQ) { // BEQ rs rt label(pc+offset<<2)
    int rs = *iter;
    iter++;
    int rt = *iter;
    iter++;

    program_num = *iter;

  } else if (opecode == BNE) { // BNE rs rt label(pc+offset<<2)
    int rs = *iter;
    iter++;
    int rt = *iter;
    iter++;

    program_num = *iter;

  } else if (opecode == J) { // J label
    program_num = *iter;

  } else if (opecode == JR) { // JR rs
    int rs = *iter;

  } else if (opecode == JAL) { // JAL label (next addr is program_num*4)

    program_num = *iter;

  } else if (opecode == JALR) { // JALR rd, rs
    int rd = *iter;
    iter++;
    int rs = *iter;

  } else if (opecode == NOP) { // nop
    program_num++;

  } else if (opecode == OUT) { // output 未対応
    program_num++;

  } else {
    program_num++;
  }
}
