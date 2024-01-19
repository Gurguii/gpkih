#pragma once
#include "subparser.hpp"
using namespace gpki;
int subparsers::init(std::vector<std::string> opts){
  // No options added yet, straight call the init function
  modes::init::create_new_profile();
  return 0;
}
