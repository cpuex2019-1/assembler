// controller.h:
// execute instructions
#ifndef EXECUTER_H
#define EXECUTER_H

#include "global.h"
#include "loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

enum Status { ACTIVE, BREAK, END };

class controller {
  loader *ld;
  Log *log_level;
  FILE *outputfile; // 出力ストリーム

  void exec_code(unsigned int line_vec);
  sim_addr get_addr_by_base_plus_offset(string base_plus_offset);
  int get_reg_num(string reg_str);
  int get_immediate(string immediate_str);

public:
  // current line number being processed
  int program_num;
  int program_addr;

  // constructor
  controller(const char *fname, loader *l, Log *l_level);

  void assemble();
};

#endif
