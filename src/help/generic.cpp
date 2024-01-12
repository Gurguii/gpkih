#include "help.hpp"

using namespace gpki;

void help::generic::usage() {
  std::cout << R"(== Gurgui public key insfraestructure helper ==
[ Generic syntax ]
  ./gpki <profile> <mode> <action> [subopts]
  e.g ./gpki testprofile build ca --keysize 4096

[ Create a new profile]
  ./gpki init

[ Building entities ] 
  - Build CA -
    ./gpki <profile> build ca
  - Build Server -
    ./gpki <profile> build server
  - Build Client -
    ./gpki <profile> build client
  )";
}
