#include "ARename.hpp"

constexpr const char *_usage = R"(== rename ==
rename profile

[ syntax ]
 ./gpki rename <profile> <newname> [subopts]

[ subopts ]
  * no specific subopts added * 
)";

constexpr const char *_examples = R"(== rename examples ==
1. Rename profile
  ./gpkih rename myProfile myNewProfileName
)";

const char *ARename::usage(){
	return _usage;
};

const char *ARename::examples(){
	return _examples;
}