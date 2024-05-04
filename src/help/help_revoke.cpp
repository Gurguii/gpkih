#include "help.hpp"

using namespace gpkih;
void help::revoke::usage() {
  fmt::print(R"(== revoke ==
revoke entitie/s

[ syntax ]
  ./gpki revoke <profile> [subopts]
  
[ subopts ]
  -s  | --serial : entity serial to revoke
  -cn | --cn     : entity common name to revoke
)");
}
