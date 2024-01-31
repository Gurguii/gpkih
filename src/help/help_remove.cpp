#include "help.hpp"

using namespace gpki;

void help::remove::usage(){
    std::cout << R"(
== remove ==

[ syntax ]
./gpki remove <profile> [subopts]

[ subopts ]

)";
}