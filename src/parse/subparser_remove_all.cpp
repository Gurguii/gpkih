#include "subparser.hpp"

using namespace gpki;

int subparsers::remove_all(){
    /* Parse subopts */
    return db::profiles::remove_all();
}