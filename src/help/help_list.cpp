#include "help.hpp"
using namespace gpki;
void help::list::usage(){
  std::cout << R"(
== list ==

[ syntax ]
  ./gpki list [profile/s] [subopts]

[ subopts ]
  -p <name> : list profile, if no name given all profiles will be printed
  -cn <commo : list entity with given common name, if no value given all entities will be printed 
  -ef <csv> : delimiter(:) entity fields to print, default all
  -pf <csv> : delimiter(:) profile fields to print, default all
)";
}
