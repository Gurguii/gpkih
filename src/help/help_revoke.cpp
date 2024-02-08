#include "help.hpp"

using namespace gpki;
void help::revoke::usage(){
  std::cout << R"(
== revoke ==
  
[ syntax ]
  ./gpki revoke <profile> <comma,separated,CN's> [subopts]
  
[ subopts ]
  --reason : revocation reason
)";
}
