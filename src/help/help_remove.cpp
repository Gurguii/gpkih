#include "help.hpp"

using namespace gpkih;

void help::remove::usage() {
  std::cout << R"(
== remove ==

[ syntax ]
 ./gpki remove <profile> [subopts]

[ subopts ]
  -all : remove all profiles instead
)";
}
