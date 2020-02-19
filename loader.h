// loader.h:
//
#ifndef LOADER_H
#define LOADER_H

#include "global.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class loader {
  // program file name
  const char *file_name;

  Log *log_level;

  // current line number
  int program_num;
  int load_line_num;
  // label
  map<string, int> label_map;
  // program
  vector<unsigned int> program_map;
  // raw program string for print
  vector<string> raw_program;

  void load_line_label(string line);
  void load_line(string line);
  int load_file();
  unsigned int format_code(vector<string>);

  int get_reg_by_base_plus_offset(string base_plus_offset);
  int get_offset_by_base_plus_offset(string base_plus_offset);
  int get_reg_num(string reg_str);
  int get_freg_num(string reg_str);
  int get_logic_immediate(string immediate_str);
  int get_arith_immediate(string immediate_str);

public:
  int end_line_num;
  // constructor
  loader(const char *fname, Log *l_level);
  // destructor
  //~loader();

  void print_binary_with_space(unsigned int v);

  int get_line_num_by_label(string label);
  int get_prog_addr_num_by_label(string label);
  unsigned int get_program_by_label(string label);
  unsigned int get_program_by_line_num(int l_num);
  string get_raw_program_by_line_num(int l_num);

  // print
  void print_label_map();
  void print_program_map();
  void print_raw_program();
};

#endif
// end of loader.h
