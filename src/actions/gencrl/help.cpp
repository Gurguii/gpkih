#include "AGencrl.hpp"

constexpr const char *_usage = R"(== gencrl ==
generate new certificate revokation list for profile

[ syntax ]
  ./gpki gencrl <profile> [subopts]

[ subopts ]
  *no subopts yet*
)";

constexpr const char *_examples = R"(== gencrl examples ==
1. Generate crl
  ./gpkih gencrl myProfile 
)";

const char *AGencrl::usage(){
	return _usage;
};

const char *AGencrl::examples(){
	return _examples;
}