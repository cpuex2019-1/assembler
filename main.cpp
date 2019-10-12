#include "controller.h"
#include "loader.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

Log log_level = DEBUG;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: %s <program name>\n", argv[0]);
    return 1;
  }

  loader *ld = new loader(argv[1], &log_level); // load program

  controller control(argv[1], ld, &log_level);
  ld->print_raw_program();

  control.assemble();
}
