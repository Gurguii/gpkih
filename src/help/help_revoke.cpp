#include "help.hpp"

using namespace gpkih;
void help::revoke::usage() {
  fmt::print(R"(== revoke ==
  
[ syntax ]
  ./gpki revoke <profile> <comma,separated,CN's> [subopts]
  
[ subopts ]
  --reason : revocation reason
)");
}
