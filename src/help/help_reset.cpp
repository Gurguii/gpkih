#include "help.hpp"
using namespace gpkih;

void help::reset::usage(){
	fmt::print(R"(== reset ==
reset gpkih, removing every profile and gpkih source dir, where database and logs are located

[ syntax ]
./gpkih reset
)");
}