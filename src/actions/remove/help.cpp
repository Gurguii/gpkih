#include "ARemove.hpp"

constexpr const char *_usage = R"(== remove ==
remove profile/s

[ syntax ]
 ./gpki remove <profile1>,<profile2>...<profileN> [subopts]

[ subopts ]
  -all : remove all profiles instead
)";

constexpr const char *_examples = R"(== remove examples ==
1. Remove profile
  ./gpkih remove myProfile
  
2. Remove all profiles
  ./gpkih remove --all
)";

const char *ARemove::usage() const {
	return _usage;
};

const char *ARemove::examples() const {
	return _examples;
}