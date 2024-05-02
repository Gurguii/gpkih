#include "help.hpp"

using namespace gpkih;

void help::remove::usage() {
  fmt::print(R"(== remove ==

[ syntax ]
 ./gpki remove <profile> [subopts]

[ subopts ]
  -all : remove all profiles instead
)");
}
