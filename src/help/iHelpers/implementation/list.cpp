#include "../iface.hpp"

namespace gpkih::help::iHelper
{
  class list : public IHelper{
  public:
    list() = default;
    void usage(bool brief = false){
  fmt::print(R"(== list ==
list profiles/entities

[ syntax ]
  ./gpkih list <profile> [subopts] | print entities from profile
  ./gpkih list [subopts]           | print all profiles

[ subopts ]
note: -cn|-ef will only affect when a profile is given, else only profiles will be printed
  
  -cn <csv> : list entity with given common name/s 
  -ef <csv> : entity fields to print, default all
  -pf <csv> : profile fields to print, default all

[ profile fields ]
  - Basic info -
  id: profile id
  name: profile name
  source: source dir

  - Dates -
  cdate | creationDate: creation
  lmod | last_modification: last modification
  
  - Entities -
  ca : ca created (bool)
  sv : server count
  cl : client count

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
  type: entity type, ca|client|server
  status: current status, active|marked|revoked
)");
}};
} // namespace