#include "help.hpp"

using namespace gpki;

void help::build::usage(){
  std::cout << R"(
== build ==
  
[ syntax ]
  ./gpki build <profile> [subopts]
  
[ subopts ]
  -ca : build certificate with ca extensions
  -sv : build certificate with sv extensions
  -cl : build certificate with cl extensions
  -keysize <int> : key size to use
  -outformat <pem|der> : output format
)";
}
