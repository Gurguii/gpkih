#include "help.hpp"

#include "texts/init.cpp"
#include "texts/build.cpp"
#include "texts/rename.cpp"
#include "texts/revoke.cpp"
#include "texts/reset.cpp"
#include "texts/remove.cpp"
#include "texts/get.cpp"
#include "texts/set.cpp"
#include "texts/list.cpp"
#include "texts/gencrl.cpp"
#include "texts/export.cpp"

using namespace gpkih;
constexpr const char *GPKIH_DESCRIPTION_BRIEF = "CLI tool to manage self-signed PKI";
constexpr const char *GPKIH_VERSION = "1.0";

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
export  | Export profile/entities' data to different storage formats

** Global flags ** (See './gpkih help flags' for details - UNIMPLEMENTED)
-y  | --yes       : autoanswer yes to questions
-q  | --quiet     : disable printing
-dr | --dryrun    : run without any modification (e.g profile changes won't get synced)
-nh | --noheader  : do not print headers when listing
-noprompt | --noprompt    : don't prompt for optional stuff (e.g creating dhparam when profile is created)
-noprint  | --noprint     : don't print certificates to stdout
--debug <int>    : print debug info to stdout, int=1-3

** Detailed help **
For extra help on any action, do:
    ./gpki help <action>
)", GPKIH_VERSION, GPKIH_DESCRIPTION_BRIEF);
}

void help::usage_brief() {
        fmt::print(R"(== Gurgui's Public Key Infraestructure helper ==

Version: {}
Description: {}

** Available actions **
init build revoke gencrl list rename get set remove reset

** Detailed help **
For extra help on any action, do:
    ./gpki help <action>
)", GPKIH_VERSION, GPKIH_DESCRIPTION_BRIEF);
}

std::unordered_map<std::string, Helper> help::helpers{
    {"init",{help::init::usage, help::init::examples}},
    {"build",{help::build::usage, help::build::examples}},
    {"revoke",{help::revoke::usage, help::revoke::examples}},
    {"list",{help::list::usage, help::list::examples}},
    {"rename",{help::rename::usage, help::rename::examples}},
    {"get",{help::get::usage, help::get::examples}},
    {"set",{help::set::usage, help::set::examples}},
    {"export",{help::exportdb::usage, help::exportdb::examples}},
    {"reset",{help::reset::usage, help::reset::examples}},
    {"remove",{help::remove::usage, help::remove::examples}},
    {"gencrl",{help::gencrl::usage, help::gencrl::examples}},
};