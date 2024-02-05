#include "parser.hpp"

using namespace gpki;

int parsers::remove_all(std::vector<str> opts){
    /* Parse subopts */
    return db::profiles::remove_all();
}