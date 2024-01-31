#include "subparser.hpp"

using namespace gpki;

int subparsers::remove_all(std::vector<str> opts){
    /* Parse subopts */
    return db::profiles::remove_all();
}