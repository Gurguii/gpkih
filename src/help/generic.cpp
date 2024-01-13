#include "help.hpp"

using namespace gpki;

void help::generic::usage() {
  std::cout << R"(== Gurgui public key insfraestructure helper ==
[ Generic syntax ]
  ./gpki <context> <action> [subopts]

[ Profile ]
  ./gpki profile add --name 'profilename1'

[ Build ] 
  - Build CA -
    ./gpki build profile1 -ca
  - Build Server -
    ./gpki build profile1 -server
  - Build Client -
    ./gpki build profile1 -client

[ Entity ]
  - List entities from profile -
    ./gpki entity profile1 -list 
  )";

}
