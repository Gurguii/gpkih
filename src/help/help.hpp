#pragma once
#include <iostream>
#include <unordered_map>
#include "../gpki.hpp"

namespace gpki::help{
void usage();  
}
namespace gpki::help::build {
void usage();
}
namespace gpki::help::revoke {
void usage();
}
namespace gpki::help::gencrl{
void usage();
}
namespace gpki::help::list{
void usage();
}
namespace gpki::help::init{
void usage();
}

using namespace gpki;
void call_helper(strview action);
