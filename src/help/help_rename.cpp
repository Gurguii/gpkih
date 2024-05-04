#include "help.hpp"
using namespace gpkih;

void help::rename::usage() {
  fmt::print(R"(== rename ==
rename profile

[ syntax ]
 ./gpki rename <profile> <newname> [subopts]

[ subopts ]
  * no specific subopts added * 
)");
}
