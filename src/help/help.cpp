#include "help.hpp"

#include "iHelpers/iface.hpp"
#include "iHelpers/implementation/init.cpp"
#include "iHelpers/implementation/build.cpp"
#include "iHelpers/implementation/rename.cpp"
#include "iHelpers/implementation/revoke.cpp"
#include "iHelpers/implementation/reset.cpp"
#include "iHelpers/implementation/remove.cpp"
#include "iHelpers/implementation/get.cpp"
#include "iHelpers/implementation/set.cpp"
#include "iHelpers/implementation/list.cpp"
#include "iHelpers/implementation/gencrl.cpp"

#include "../gpkih.hpp"

#include <stdexcept>
#include <map>
#include <tuple>
using namespace gpkih;

int help::callHelper(std::string_view msg){
    if(msg == "init"){
        static iHelper::init _h{};
        _h.usage();
    }else if(msg == "build"){
        static iHelper::build _h{};
        _h.usage();
    }else if(msg == "list"){
        static iHelper::list _h{};
        _h.usage();
    }else if(msg == "revoke"){
        static iHelper::revoke _h{};
        _h.usage();
    }else if(msg == "rename"){
        static iHelper::rename _h{};
        _h.usage();
    }else if(msg == "gencrl"){
        static iHelper::gencrl _h{};
        _h.usage();
    }else if(msg == "set"){
        static iHelper::set _h{};
        _h.usage();
    }else if(msg == "get"){
        static iHelper::get _h{};
        _h.usage();
    }else if(msg == "reset"){
        static iHelper::reset _h{};
        _h.usage();
    }else if(msg == "remove"){
        static iHelper::remove _h{};
        _h.usage();
    }
	return 0;
}

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
-y  | --yes       : autoanswer yes to questions
-q  | --quiet     : disable printing
-dr | --dryrun    : run without any modification (e.g profile changes won't get synced)
-nh | --noheader  : 
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