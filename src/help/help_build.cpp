#include "help.hpp"

using namespace gpkih;

void help::build::usage() {
  std::cout << R"(
== build ==
  
[ syntax ]
  ./gpki build *<profile> *<entity_type> [subopts]
  
[ entity types ]
  ca : ca
  sv : server
  cl : client

[ subopts ]
  -keysize <int> : key size to use
  -outformat <pem|der> : output format
)";
}
