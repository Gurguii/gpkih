#include "help.hpp"

using namespace gpki;
void help::revoke::usage(){
  std::cout << R"(
== revoke ==
  
[ syntax ]
  ./gpki revoke <profile> [subopts]
  
[ subopts ]
  -cn : specify entity common name
  -reason : revocation reason
)";
}
