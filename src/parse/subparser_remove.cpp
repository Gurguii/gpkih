#include "subparser.hpp"

using namespace gpki;

int subparsers::remove(Profile *profile, std::vector<std::string> opts){
    /* Parse subopts */
    return db::profiles::remove(profile->name);
}