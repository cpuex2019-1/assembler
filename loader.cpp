// loader.cpp:
//

#include "loader.h"
#include "asm.h"
#include "global.h"
#include <regex>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

string label_str_tmp;

// constructor
loader::loader(const char *fname, Log *l_level) {
  file_name = fname;
  log_level = l_level;
  load_line_num = 0;
  program_num = 0; // reset line number

  end_line_num = load_file();
}

// destructor
/*
loader::~loader() {

}
*/
int loader::get_reg_by_base_plus_offset(string base_plus_offset) {
  regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
  sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                             sep, 3);
  sregex_token_iterator end;
  if (iter == end) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n", load_line_num,
             base_plus_offset.c_str());
    }
    exit(1);
  } else {
    string base_str = iter->str();
    int reg_num = 0;
    try {
      reg_num = stoi(base_str);
      return reg_num;
    } catch (std::out_of_range &e) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid base plus offset: "
               "[%s](out_of_range)\n",
               load_line_num, base_plus_offset.c_str());
      }
      exit(1);
    } catch (...) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
               load_line_num, base_plus_offset.c_str());
      }
      exit(1);
    }
  }
}
int loader::get_offset_by_base_plus_offset(string base_plus_offset) {
  regex sep("^([+-]?)(0|[1-9][0-9]*)\\(\\$(3[0-1]|[1-2][0-9]|[0-9])\\)$");
  sregex_token_iterator iter(base_plus_offset.begin(), base_plus_offset.end(),
                             sep, {1, 2});
  sregex_token_iterator end;
  if (iter == end) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tinvalid base plus offset[%s]\n", load_line_num,
             base_plus_offset.c_str());
    }
    exit(1);
  } else {
    string sign = iter->str();
    iter++;
    string offset_str = iter->str();
    int offset = 0;
    try {
      offset = stoi(offset_str); // convert string to int
      if (sign == "-") {
        return -offset;
      } else {
        return offset;
      }
    } catch (std::out_of_range &e) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid base plus offset: "
               "[%s](out_of_range)\n",
               load_line_num, base_plus_offset.c_str());
      }
      exit(1);
    } catch (...) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid base plus offset: [%s]\n",
               load_line_num, base_plus_offset.c_str());
      }
      exit(1);
    }
  }
}

int loader::get_reg_num(string reg_str) {
  regex sep("^\\$(3[0-1]|[1-2][0-9]|[0-9])$");
  sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
  sregex_token_iterator end;
  if (iter == end) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
             reg_str.c_str());
    }
    exit(1);
  } else {
    try {
      int reg_num = stoi(iter->str()); // convert string to int
      return reg_num;
    } catch (std::out_of_range &e) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid register: "
               "[%s](out_of_range)\n",
               load_line_num, reg_str.c_str());
      }
      exit(1);
    } catch (...) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid register: [%s]\n", load_line_num,
               reg_str.c_str());
      }
      exit(1);
    }
  }
}

int loader::get_freg_num(string reg_str) {
  regex sep("^\\$f(3[0-1]|[1-2][0-9]|[0-9])$");
  sregex_token_iterator iter(reg_str.begin(), reg_str.end(), sep, 1);
  sregex_token_iterator end;
  if (iter == end) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tinvalid register[%s]\n", load_line_num,
             reg_str.c_str());
    }
    exit(1);
  } else {
    try {
      int reg_num = stoi(iter->str()); // convert string to int
      return reg_num;
    } catch (std::out_of_range &e) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid register: "
               "[%s](out_of_range)\n",
               load_line_num, reg_str.c_str());
      }
      exit(1);
    } catch (...) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid register: [%s]\n", load_line_num,
               reg_str.c_str());
      }
      exit(1);
    }
  }
}

int loader::get_arith_immediate(string init_immediate_str) {
  // check immediate
  string immediate_str = init_immediate_str;
  regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
  sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                             {1, 2});
  sregex_token_iterator end;
  if (iter != end) {
    string sign = iter->str();
    iter++;
    if (iter == end) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
               immediate_str.c_str());
      }
      exit(1);
    } else {
      try {
        int immediate =
            stoi(iter->str()); // convert string to int to unsigned int
        if (sign == "-") {
          if (immediate <= 32768) { // -2^15まで
            return -immediate;
          } else {
            if (*log_level >= FATAL) {
              printf("FATAL\tline:%d\tinvalid immediate: "
                     "[%s](under -2^15)\n",
                     load_line_num, immediate_str.c_str());
            }
            exit(1);
          }
        } else {
          if (immediate <= 32767) { // 2^15-1まで
            return immediate;
          } else {
            if (*log_level >= FATAL) {
              printf("FATAL\tline:%d\tinvalid immediate: "
                     "[%s](over 2^15-1)\n",
                     load_line_num, immediate_str.c_str());
            }
            exit(1);
          }
        }

      } catch (std::out_of_range &e) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate: "
                 "[%s](out_of_range)\n",
                 load_line_num, immediate_str.c_str());
        }
        exit(1);
      } catch (...) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate: [%s]\n", load_line_num,
                 immediate_str.c_str());
        }
        exit(1);
      }
    }
  } else { // ha(label) or lo(label)
    immediate_str = init_immediate_str;
    regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                               halo_regex, {1, 2});
    sregex_token_iterator end;
    if (iter != end) {

      string halo = iter->str();
      iter++;
      if (iter == end) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                 immediate_str.c_str());
        }
        exit(1);
      } else {
        string label_str = iter->str(); // convert string to int to unsigned int
        int label_num = get_line_num_by_label(label_str);
        unsigned int label_addr = ((unsigned int)label_num) * 4;

        if (halo == "ha") {
          return label_addr >> 16;
        } else if (halo == "lo") {
          return (label_addr << 16) >> 16;
        } else {
          if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid immediate: [%s]\n", load_line_num,
                   immediate_str.c_str());
          }
          exit(1);
        }
      }
    } else {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
               immediate_str.c_str());
      }
      exit(1);
    }
  }
}

int loader::get_logic_immediate(string init_immediate_str) {
  // check immediate
  string immediate_str = init_immediate_str;
  regex sep("^([+-]?)(0|[1-9][0-9]*)$"); //([+-]?)([0-9]+)
  sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(), sep,
                             {1, 2});
  sregex_token_iterator end;
  if (iter != end) {
    string sign = iter->str();
    iter++;
    if (iter == end) {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
               immediate_str.c_str());
      }
      exit(1);
    } else {
      try {
        int immediate =
            stoi(iter->str()); // convert string to int to unsigned int

        if (sign == "-") {
          if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid immediate: "
                   "[%s](under 0)\n",
                   load_line_num, immediate_str.c_str());
          }
          exit(1);
        } else {
          if (immediate <= 65535) { // 2^16-1まで
            return immediate;
          } else {
            if (*log_level >= FATAL) {
              printf("FATAL\tline:%d\tinvalid immediate: "
                     "[%s](over 2^16-1)\n",
                     load_line_num, immediate_str.c_str());
            }
            exit(1);
          }
        }
      } catch (std::out_of_range &e) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate: "
                 "[%s](out_of_range)\n",
                 load_line_num, immediate_str.c_str());
        }
        exit(1);
      } catch (...) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate: [%s]\n", load_line_num,
                 immediate_str.c_str());
        }
        exit(1);
      }
    }
  } else { // ha(label) or lo(label)
    immediate_str = init_immediate_str;
    regex halo_regex("^(ha|lo)\\(([A-Za-z][\\w\\.]*)\\)$");
    sregex_token_iterator iter(immediate_str.begin(), immediate_str.end(),
                               halo_regex, {1, 2});
    sregex_token_iterator end;
    if (iter != end) {

      string halo = iter->str();
      iter++;
      if (iter == end) {
        if (*log_level >= FATAL) {
          printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
                 immediate_str.c_str());
        }
        exit(1);
      } else {
        string label_str = iter->str(); // convert string to int to unsigned int
        int label_num = get_line_num_by_label(label_str);
        unsigned int label_addr = ((unsigned int)label_num) * 4;

        if (halo == "ha") {
          return label_addr >> 16;
        } else if (halo == "lo") {
          return (label_addr << 16) >> 16;
        } else {
          if (*log_level >= FATAL) {
            printf("FATAL\tline:%d\tinvalid immediate: [%s]\n", load_line_num,
                   immediate_str.c_str());
          }
          exit(1);
        }
      }
    } else {
      if (*log_level >= FATAL) {
        printf("FATAL\tline:%d\tinvalid immediate[%s]\n", load_line_num,
               immediate_str.c_str());
      }
      exit(1);
    }
  }
}

int loader::load_file() {
  // load label
  ifstream input;
  input.open(file_name);
  if (!input) {
    if (*log_level >= FATAL) {
      printf("can't open file: %s\n", file_name);
    }
    exit(1);
  }
  string linebuf;
  while (!input.eof()) {
    load_line_num++;
    getline(input, linebuf);
    load_line_label(linebuf);
  }
  if (input.is_open()) {
    input.close();
  }

  // load program
  program_num = 0;
  load_line_num = 0;
  input.open(file_name);
  if (!input) {
    if (*log_level >= FATAL) {
      printf("can't open file: %s\n", file_name);
    }
    exit(1);
  }
  while (!input.eof()) {
    load_line_num++;
    getline(input, linebuf);
    load_line(linebuf);
  }
  if (input.is_open()) {
    input.close();
  }

  return program_num;
}

void loader::load_line_label(string line) {
  // delete comment
  regex comment_pattern("#.*$"); // #から末尾まで
  sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
  sregex_token_iterator end1;
  string line_not_comment = "";
  for (; iter1 != end1; iter1++) {
    line_not_comment = line_not_comment + iter1->str();
  }

  // get label
  regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
  sregex_token_iterator iter2(line_not_comment.begin(), line_not_comment.end(),
                              label_pattern,
                              {1, -1}); // group1: label, 残り: コード
  sregex_token_iterator end;
  string label_str = iter2->str();
  iter2++;
  string res_str = "";
  for (; iter2 != end; iter2++) {
    res_str = res_str + iter2->str();
  }

  // remove redundant spases
  vector<string> code;
  regex sep1("[\\t ]+");
  sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
  string res = "";
  for (; iter3 != end; iter3++) {
    res = res + iter3->str();
  }

  if (res != "") {
    program_num++; // increment before processing the line
  }

  if (label_str != "") {
    label_map.insert(std::make_pair(label_str, program_num));
  }
}

// load line
void loader::load_line(string line) {

  // delete comment
  regex comment_pattern("#.*$"); // #から末尾まで
  sregex_token_iterator iter1(line.begin(), line.end(), comment_pattern, -1);
  sregex_token_iterator end1;
  string line_not_comment = "";
  for (; iter1 != end1; iter1++) {
    line_not_comment = line_not_comment + iter1->str();
  }

  // get label
  regex label_pattern("^[\\t ]*(?:([A-Za-z][\\w\\.]*)[:])?[\\t ]*");
  sregex_token_iterator iter2(line_not_comment.begin(), line_not_comment.end(),
                              label_pattern,
                              {1, -1}); // group1: label, 残り: コード
  sregex_token_iterator end;
  string label_str = iter2->str();
  iter2++;
  string res_str = "";
  for (; iter2 != end; iter2++) {
    res_str = res_str + iter2->str();
  }
  if (label_str != "") {
    label_str_tmp = label_str;
  }
  // split opecode and residual and remove redundant spases
  vector<string> code;
  regex sep1("[\\t ]+");
  sregex_token_iterator iter3(res_str.begin(), res_str.end(), sep1, -1);
  string opecode_str = iter3->str();
  code.push_back(opecode_str);
  iter3++;
  string res = "";
  for (; iter3 != end; iter3++) {
    res = res + iter3->str();
  }

  // process operand (split res by ",")
  regex sep2(",");
  sregex_token_iterator iter4(res.begin(), res.end(), sep2, -1);
  for (; iter4 != end; iter4++) {
    code.push_back(iter4->str());
  }

  if (code[0] != "") {
    if (label_str_tmp != "") {
      raw_program.push_back(label_str_tmp + ":\t" + opecode_str + "\t" + res);
    } else {
      raw_program.push_back("\t\t" + opecode_str + "\t" + res);
    }
    vector<int> formatted_code = format_code(code);
    program_map.push_back(formatted_code);
    label_str_tmp = "";
    program_num++; // increment before processing the line
  }
}

vector<int> loader::format_code(vector<string> code) {
  auto iter = code.begin();
  string opecode = *iter;
  iter++;
  vector<int> result;

  if (opecode == "add") { // ADD rd <- rs + rt
    result.push_back(ADD);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "addi") { // ADDI rd <- rs + immediate
    result.push_back(ADDI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_arith_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sub") { // SUB rd <- rs - rt
    result.push_back(SUB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "mul") { // MUL rd <- rs * rt
    result.push_back(MUL);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "div10") { // DIV10 rd <- rs / 10
    result.push_back(DIV10);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "mod") { // MOD rd <- rs % rt
    result.push_back(MOD);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "slt") { // SLT Rd = if Rs < Rt then 1 else 0
    result.push_back(SLT);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "and") { // AND rd <- rs & rt
    result.push_back(AND);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "andi") { // ADNI rd <- rs & immediate
    result.push_back(ANDI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "or") { // OR rd <- rs | rt
    result.push_back(OR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "ori") { // ORI rd <- rs & immediate
    result.push_back(ORI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "nor") { // NOR rd <- ~(rs | rt)
    result.push_back(NOR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "xor") { // XOR rd <- rs ^ rt
    result.push_back(XOR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "xori") { // XORI rd <- rs & immediate
    result.push_back(XORI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "srai") { // SRAI rd <- rs >> sb (arithmetic)
    result.push_back(SRAI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "srli") { // SRLI rd <- rs >> sb (logical)
    result.push_back(SRLI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "slli") { // SLLI rd <- rs << sb (logical)
    result.push_back(SLLI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int immediate = get_logic_immediate(*iter);
        result.push_back(immediate);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sra") { // SRA rd <- rs >> rt (arithmetic)
    result.push_back(SRA);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "srl") { // SRL rd <- rs >> rt (logical)
    result.push_back(SRL);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sll") { // SLLI rd <- rs << rt (logical)
    result.push_back(SLL);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_reg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "itof") { // ITOF rd <- itof(rs)
    // rd is a float registor
    result.push_back(ITOF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fadd") { // FADD rd <- rs +. rt
    result.push_back(FADD);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fsub") { // FSUB rd <- rs -. rt
    result.push_back(FSUB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fmul") { // FMUL rd <- rs *. rt
    result.push_back(FMUL);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fdiv") { // FDIV rd <- rs /. rt
    result.push_back(FDIV);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sqrt") { // SQRT rd <- sqrt(rs)
    result.push_back(SQRT);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sin") { // SIN rd <- sin(rs)
    result.push_back(SIN);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "cos") { // COS rd <- cos(rs)
    result.push_back(COS);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "atan") { // ATAN rd <- atan(rs)
    result.push_back(ATAN);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sltf") { // SLTF Rd = if Rs < Rt then 1 else 0
    // *rd is a general register
    result.push_back(SLTF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fneg") { // SQRT rd <- - rs
    result.push_back(FNEG);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "fabs") { // FABS rd <- abs(rs)
    result.push_back(FABS);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "floor") { // FLOOR rd <- floor(rs)
    result.push_back(FLOOR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "ftoi") { // FTOI rd <- ftoi(rs)
    // rd is a general register
    result.push_back(FTOI);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "lw") { // LW rd, offset(base)
    result.push_back(LW);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "lb") { // LB rd, offset(base)
    result.push_back(LB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sw") {
    result.push_back(SW);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sb") { // sb rd, offset(base)
    result.push_back(SB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "mov" || opecode == "move") { // mov rd <- rs
    result.push_back(MOV);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "lf") { // lf rd, offset(base)
    result.push_back(LF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "sf") { // SF rt, offset(base)
    result.push_back(SF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rt = get_freg_num(*iter);
        result.push_back(rt);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int reg = get_reg_by_base_plus_offset(*iter);
        result.push_back(reg);
        int offset = get_offset_by_base_plus_offset(*iter);
        result.push_back(offset);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "movf") { // MOVF rd <- rs
    result.push_back(MOVF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "bc") { // BC label(pc+offset<<2)
    result.push_back(BC);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num - program_num);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "beq") { // BEQ rs rt label(pc+offset<<2)
    result.push_back(BEQ);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num - program_num);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "bne") { // BNE rs rt label(pc+offset<<2)
    result.push_back(BNE);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter == code.end()) {
        throw 3;
      } else {
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num - program_num);
        iter++;
      }
      if (iter != code.end()) {
        throw 4;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "j") { // J label
    result.push_back(J);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "jr") { // JR rs
    result.push_back(JR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode ==
             "jal") { // JAL label (next instruction addr is program_num*4)
    result.push_back(JAL);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        string label_str = *iter;
        int label_num = get_line_num_by_label(label_str);
        result.push_back(label_num);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "jalr") { // JALR rd, rs
    result.push_back(JALR);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter == code.end()) {
        throw 2;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 3;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "inb") { // INB rd
    result.push_back(INB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "in") { // IN rd
    result.push_back(IN);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_reg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "outb") { // OUTB rs
    result.push_back(OUTB);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "out") { // OUT rs
    result.push_back(OUT);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rs = get_reg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "inf") { // INF rd
    result.push_back(INF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rd = get_freg_num(*iter);
        result.push_back(rd);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "outf") { // OUTF rs
    result.push_back(OUTF);
    try {
      if (iter == code.end()) {
        throw 1;
      } else {
        int rs = get_freg_num(*iter);
        result.push_back(rs);
        iter++;
      }
      if (iter != code.end()) {
        throw 2;
      }
    } catch (int arg_num) {
      printf("FATAL\tline:%d\tinvalid argument%d: [%s]\n", load_line_num,
             arg_num, get_raw_program_by_line_num(program_num).c_str());
      exit(1);
    }

  } else if (opecode == "nop") { // nop
    result.push_back(NOP);

  } else {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tinvalid opecode:%s\n", load_line_num,
             opecode.c_str());
    }
    exit(1);
  }
  return result;
}

// public

int loader::get_line_num_by_label(string label) {
  auto it = label_map.find(label);
  if (it == label_map.end()) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tnot found label: %s\n", load_line_num,
             label.c_str());
    }
    exit(1);
  } else {
    return label_map[label];
  }
}

int loader::get_prog_addr_num_by_label(string label) {
  auto it = label_map.find(label);
  if (it == label_map.end()) {
    if (*log_level >= FATAL) {
      printf("FATAL\tline:%d\tnot found label: %s\n", load_line_num,
             label.c_str());
    }
    exit(1);
  } else {
    return label_map[label] * 4;
  }
}

vector<int> loader::get_program_by_label(string label) {
  int line_num_of_label = get_line_num_by_label(label);
  return program_map[line_num_of_label];
}

vector<int> loader::get_program_by_line_num(int l_num) {
  return program_map[l_num];
}

string loader::get_raw_program_by_line_num(int l_num) {
  return raw_program[l_num];
}

void loader::print_label_map() {
  printf("label map\n");
  for (auto itr = label_map.begin(); itr != label_map.end(); ++itr) {
    printf("\t%s :\t%d\n", itr->first.c_str(), (itr->second) * 4);
  }
}
void loader::print_program_map() {
  printf("program map\n");
  int line = 0;
  for (auto itr = program_map.begin(); itr != program_map.end(); ++itr) {
    for (auto itr_int = itr->begin(); itr_int != itr->end(); ++itr_int) {
      printf("%d\t", *itr_int);
    }
    printf("\n");

    line++;
  }
}

void loader::print_raw_program() {
  printf("instruction memory\n");
  int line = 0;
  for (auto itr = raw_program.begin(); itr != raw_program.end(); ++itr) {
    printf("%8d:\t%s\n", line * 4, itr->c_str());
    line++;
  }
}
