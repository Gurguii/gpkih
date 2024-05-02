#include "help.hpp"

using namespace gpkih;

void help::set::usage()
{
	fmt::print(R"(== set ==
		
[ syntax ]
  ./gpkih set <profile> <file>.<section>.<property>=<new_value>
)");
}