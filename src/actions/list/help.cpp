#include "AList.hpp"

constexpr const char *_usage = R"(== list ==
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
  lmod | lastModification: last modification
  
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
)";

constexpr const char *_examples = R"(
1. List profiles
  ./gpkih list
2. List entities from profile
  ./gpkih list myProfile
3. List entities from profile, display cn, creation date, and status
  ./gpkih list myProfile -f cn,cdate,status
4. List profiles, display name creation date, last modification date
  ./gpkih list -f name,cdate,lmod
)";

const char *AList::usage() const {
	return _usage;
};

const char *AList::examples() const {
	return _examples;
}