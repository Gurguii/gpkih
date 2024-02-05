#include "help.hpp"

using namespace gpki;
void help::get::usage(){
    std::cout << R"(
== get ==

[ syntax ]
  ./gpki get <profile> prop1 prop2 ... propN
  ./gpki get test client.remote  server.verbose
)";
}