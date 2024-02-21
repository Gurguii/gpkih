#include "help.hpp"

using namespace gpkih;
void help::set::usage() {
  fmt::print(R"(
== set ==

[ syntax ]
  ./gpki set <profile> prop1=val1 prop2=val2  ... propN=valN
  ./gpki set test client.remote='24.24.24.24 9999'  server.verbose=1
)");
}
