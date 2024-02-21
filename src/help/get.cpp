#include "help.hpp"

using namespace gpkih;
void help::get::usage() {
  fmt::print(R"(
== get ==

[ syntax ]
  ./gpki get <profile> prop1 prop2 ... propN
  ./gpki get test client.remote  server.verbose
)");
}
