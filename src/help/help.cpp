#include "help.hpp"
#include <unordered_map>

using namespace gpkih;

static constexpr const char *VERSION = "1.0";
static constexpr const char *AUTHOR = "Airán 'gurgui' Gómez";
static constexpr const char *DESCRIPTION = "Simple cli tool to create and manage self signed PKI";

#ifdef GPKIH_ENABLE_DEBUGGING
constexpr const char *debugHelpOption = "--debug <int> : print debug info to stdout, int=1-3\n";
#else
constexpr const char *debugHelpOption = "\n";
#endif

void help::usage() {
  fmt::print(R"(== Gurgui's Public Key Infraestructure helper ==

Version: {}
Description: {}

** Actions **
init    | Create a new profile 
build   | Create certificates
revoke  | Revoke certificates
gencrl  | Generate crl (certificate revocation list)
list    | List profiles and entities
remove  | Remove profile
rename  | Rename profile
reset   | Reset gpkih (remove all logs + profiles)
get     | Inspect global/profile-specific configuration
set     | Modify global/profile-specific configuration


** Global flags ** (See './gpkih help flags' for details - UNIMPLEMENTED)
-noprompt | --noprompt    : don't prompt for optional stuff (e.g creating dhparam when profile is created)
-noprint  | --noprint     : don't print certificates to stdout
-y  | --yes      : autoanswer yes to questions
-dr | --dryrun   : do not make changes
-nh | --noheader : do not print headers
{}

** Detailed help **
For extra help on any action, do:
    ./gpki help <action>
)", VERSION, DESCRIPTION,debugHelpOption);
}

void help::usage_brief() {
        fmt::print(R"(== Gurgui's Public Key Infraestructure helper ==

Version: {}
Description {}

** Available actions **
init build revoke gencrl list rename get set remove reset 

** Detailed help **
For extra help on any action, do:
    ./gpki help <action>
)", VERSION, DESCRIPTION);
}

std::unordered_map<std::string, void(*)()> help::help_funcs{
  {"init"  , gpkih::help::init::usage},
  {"build" , gpkih::help::build::usage},
  {"list"  , gpkih::help::list::usage},
  {"revoke", gpkih::help::revoke::usage},
  {"gencrl", gpkih::help::gencrl::usage},
  {"remove", gpkih::help::remove::usage},
  {"set"   , gpkih::help::set::usage},
  {"get"   , gpkih::help::get::usage},
  {"rename", gpkih::help::rename::usage},
  {"reset" , gpkih::help::reset::usage}
};

void help::call_helper(std::string_view action){
  if(help_funcs.find(action.data()) != help_funcs.end()){
    // usage() functions exists for given action, call it
    help_funcs[action.data()]();
  }else{
    PERROR("no usage defined for action '{}'\n", action);
  }
}