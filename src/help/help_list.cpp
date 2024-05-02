#include "help.hpp"
using namespace gpkih;
void help::list::usage() {
  fmt::print(R"(== list ==

[ syntax ]
  ./gpki list [profile/s] [subopts]

note: profile name/s can be omitted, in that case 
all profiles will be assumed

[ subopts ]
  -cn <common_name> : list entity with given common name, if no value given all entities will be printed 
  -ef <csv> : delimiter(,) entity fields to print, default all
  -pf <csv> : delimiter(,) profile fields to print, default all

[ profile fields ]
 name: profile name
 source: source dir

[ entity fields ]
  - Subject -
  cn: common name
  country: country
  state: state
  location: location
  org: org
  mail: email
  
  - Paths -
  key: path to key 
  req: path to certificate request
  crt: path to certificate

  - Other -
  type: the type of entity, ca|client|server
)");
}
