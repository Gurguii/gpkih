#include "help.hpp"

using namespace gpkih;

void help::create_pack::usage() {
  fmt::print(R"(
== create_pack ==

[ syntax ]
    ./gpki create-pack [profiles] [cn1,cn2...cnX] [subopts]

[ subopts ]
    -o | --out : output dir (a copy will always be saved in the profile dir)
    --inline : output file in inline format (1 single config file)
 )");
}
