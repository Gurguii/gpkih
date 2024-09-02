#include "AInit.hpp"

constexpr const char *_usage = R"(== init ==
Create a new profile with its according PKI file structure

[ syntax ]
  ./gpki init [subopts]

[ subopts ]
  -n | --name : profile name
  -s | --source : source dir (where profile related files will be kept
)";

constexpr const char *_examples = R"(== init examples ==
1. Create a new profile interactively
  ./gpkih init
2. Create a profile without being prompted for dhparam/CA creation
  ./gpkih init -n myProfile -s ~/MyProfileDir --noprompt
)";

const char *AInit::usage() const {
	return _usage;
};

const char *AInit::examples() const {
	return _examples;
}