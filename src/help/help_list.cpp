#include "help.hpp"
using namespace gpkih;
void help::list::usage() {
  fmt::print(R"(
== list ==

[ syntax ]
  ./gpki list [profile/s] [subopts]

[ subopts ]
  -cn <common_name> : list entity with given common name, if no value given all entities will be printed 
  -ef <csv> : delimiter(,) entity fields to print, default all
  -pf <csv> : delimiter(,) profile fields to print, default all

[ profile fields ]
 name: profile name
 source: source dir

[ entity fields ]
  - Subject -
  cn:  subject common name
  country: subject country
  state: subject state
  location: subject location
  org: subject org
  mail: subject email
  - Paths -
  key: path to key 
  req: path to request (csr)
  crt: path to certificate
  - Other -
  type: the type of entity, ca|client|server
)");
}
