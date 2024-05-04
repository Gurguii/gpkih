#include "help.hpp"

using namespace gpkih;
void help::init::usage() {
  fmt::print(R"(== init ==
Create a new profile with its according PKI file structure

[ syntax ]
  ./gpki init [subopts]

[ subopts ]
  -n | --name : profile name
  -s | --source : source dir (where profile related files will be kept)
)");
}
