#include "AReset.hpp"

constexpr const char *_name = "reset";

constexpr const char *_usage = R"(== reset ==
reset gpkih, removing every profile and gpkih source dir, where database and logs are located

[ syntax ]
./gpkih reset
)";

constexpr const char *_examples = R"(== reset examples ==
1. Reset gpkih without confirmation
  ./gpkih reset -y
)";

const char *AReset::usage(){
	return _usage;
};

const char *AReset::examples(){
	return _examples;
}