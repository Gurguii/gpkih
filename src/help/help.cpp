#include "help.hpp"
#include <stdio.h>

using namespace gpkih;
constexpr const char *GPKIH_DESCRIPTION_BRIEF = "CLI tool to manage self-signed PKI's";
constexpr const char *GPKIH_VERSION = "1.0";

void help::usage() {
  printf(R"(== Gurgui's Public Key Infraestructure helper ==

Version: %s
Description: %s

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
dhparam | Do stuff with diffie-hellman parameters

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
    printf(R"(== Gurgui's Public Key Infraestructure helper ==

Version: %s
Description: %s

** Available actions **
init build revoke gencrl list rename get set remove reset

** Detailed help **
For extra help on any action, do:
    ./gpki help <action>
)", GPKIH_VERSION, GPKIH_DESCRIPTION_BRIEF);
}