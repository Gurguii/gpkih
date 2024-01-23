#include "help_build.cpp"
#include "help_revoke.cpp"
#include "help_gencrl.cpp"
#include "help_list.cpp"
#include "help_init.cpp"
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
  ./gpki list [subopts]

[build] 
Create certificates
  ./gpki build [profile] [subopts]

[revoke]
Revoke certicates
  ./gpki revoke [profile] [subopts]

[gencrl]
Generate crl
  ./gpki gencrl [profile] [subopts]
)";

}