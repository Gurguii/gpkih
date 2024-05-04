#include "help.hpp"

using namespace gpkih;

void help::set::usage()
{
	fmt::print(R"(== set ==
UNIMPLEMENTED
set generic/profile-specific configuration

[ syntax ]
  ./gpkih set <profile> <file>.<section>.<property>=<new_value>
)");
}