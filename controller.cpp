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
  program_addr = 0;

  string filename = fname;
  filename.pop_back(); // 最後のsを削除
  filename = filename + "coe";
  outputfile = fopen(filename.c_str(), "w");
  if (outputfile == NULL) { // オープンに失敗した場合
    printf("cannot open file\n");
    exit(1);
  }
  string tmp = fname;
  tmp.pop_back(); // 最後のsを削除
  tmp.pop_back(); // 最後の.を削除
  fprintf(
      outputfile,
      "; %s\nmemory_initialization_radix=16;\nmemory_initialization_vector=\n",
      tmp.c_str());
}

void controller::assemble() {

  while (program_num < ld->end_line_num) {
    unsigned int one_code = ld->get_program_by_line_num(program_num);
    if (*log_level >= INFO) {
      // raw_programを出力
      string one_raw_program = ld->get_raw_program_by_line_num(program_num);
      printf("INFO\t%d:\t%s", program_num * 4, one_raw_program.c_str());
      if (*log_level >= TRACE) {
        printf("\t(TRACE\t");
        ld->print_binary_with_space(one_code);
        printf(")");
      }
      printf("\n");
    }

    exec_code(one_code);

    program_num++;
    program_addr += 4;

    if (program_num >= ld->end_line_num) {
      fprintf(outputfile, ";\n");
    } else {
      fprintf(outputfile, ",\n");
    }
  }
  fclose(outputfile);
}

void controller::exec_code(unsigned int one_code) {

  if (*log_level >= DEBUG) {
    printf("hex(16):%08x\tbinary:", one_code);
    print_binary(one_code);
    printf("\n");
  }

  fprintf(outputfile, "%08x", one_code);
}
