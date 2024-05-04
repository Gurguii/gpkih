#include "help.hpp"

using namespace gpkih;
void help::gencrl::usage() {
  fmt::print(R"(== gencrl ==
generate new certificate revokation list for profile

[ syntax ]
  ./gpki gencrl <profile> [subopts]

[ subopts ]
  *no subopts yet*
)");
}
