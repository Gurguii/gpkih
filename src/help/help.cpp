//#include "get.cpp"
//#include "help_build.cpp"
//#include "help_create_pack.cpp"
//#include "help_gencrl.cpp"
//#include "help_init.cpp"
//#include "help_list.cpp"
//#include "help_remove.cpp"
//#include "set.cpp"
#include "help.hpp"
#include <unordered_map>

using namespace gpkih;
void help::usage() {
  fmt::print(R"(
== Public key infraestructure helper ==

Author: {} 
Description: {}
Version: {}

For extra help on any action, do:
    ./gpki help [action]

[init]
Used to create a new profile
  ./gpki init [subopts]

[list]
Used to display info about profiles and entities
  ./gpki list [profile/s] [subopts]

[build] 
Create certificates
  ./gpki build [profile] [subopts]

[revoke]
Revoke certicates
  ./gpki revoke [profile/s] [subopts]

[gencrl]
Generate crl
  ./gpki gencrl [profile] [subopts]

[ remove ]
Remove profile files and database entry
  ./gpki remove [profile/s]
)", Config::get("metadata", "author"), Config::get("metadata", "description"), Config::get("metadata", "version"));
}

std::unordered_map<str, void(*)()> help::help_funcs{
  {"init"  , gpkih::help::init::usage},
  {"build" , gpkih::help::build::usage},
  {"list"  , gpkih::help::list::usage},
  {"revoke", gpkih::help::revoke::usage},
  {"gencrl", gpkih::help::gencrl::usage},
  {"remove", gpkih::help::remove::usage}
};

void help::call_helper(strview action){
  if(help_funcs.find(action.data()) != help_funcs.end()){
    // usage() functions exists for given action, call it
    help_funcs[action.data()]();
  }else{
    PERROR("no help::usage() defined for action '{}'\n", action);
  }
}

void help::usage_brief() {
        fmt::print(R"(
== Public key infraestructure helper ==

Author: {} 
Description: {}
Version: {}

Actions: init build revoke gencrl list remove

For extra help on any action, do:
    ./gpki help [action]
)", Config::get("metadata", "author"), Config::get("metadata", "description"), Config::get("metadata", "version"));
}
