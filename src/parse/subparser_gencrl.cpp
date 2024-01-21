#include "subparser.hpp"
using namespace gpki;
int subparsers::gencrl(Profile *profile, std::vector<std::string> opts){
  subopts::gencrl params;
  return actions::gencrl(profile,&params);
}
