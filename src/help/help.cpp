#include "help.hpp"
#include "get.cpp"
#include "help_build.cpp"
#include "help_create_pack.cpp"
#include "help_gencrl.cpp"
#include "help_init.cpp"
#include "help_list.cpp"
#include "help_remove.cpp"
#include "help_revoke.cpp"
#include "set.cpp"

void help::usage() {
  std::cout << R"(
== Public key infraestructure helper ==

Author: Airán 'Gurgui' Gómez 
Description: Tool to create and manage PKI profiles, 
manage certificates and some other stuff

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

[ set ]
Change profile's vpn configuration
  ./gpki set [profile] prop1=val1 prop2=val2 ... propN=valN

[ get ]
Get profile vpn configuration
  ./gpki get [profile] prop1 prop2 ... propN
)";
}

void call_helper(strview action) {
  if (action == "build") {
    help::build::usage();
  } else if (action == "revoke") {
    help::revoke::usage();
  } else if (action == "init") {
    help::init::usage();
  } else if (action == "gencrl") {
    help::gencrl::usage();
  } else if (action == "list") {
    help::list::usage();
  } else if (action == "remove") {
    help::remove::usage();
  } else if (action == "create-pack") {
    help::create_pack::usage();
  } else if (action == "set") {
    help::set::usage();
  } else if (action == "get") {
    help::get::usage();
  } else {
    PERROR("no help defined for '{}'\n", action);
  }
}
