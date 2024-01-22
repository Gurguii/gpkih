#include "help.hpp"

using namespace gpki;
void help::init::usage(){
  std::cout << R"(
== init ==

[ syntax ]
  ./gpki init [subopts]

[ subopts ]
  -n | --name : profile name
  -s | --source : source dir (where profile related files will be kept)
)";
}
