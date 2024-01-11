#include "gpki.hpp"
#include "db/entities.cpp"
#include "db/profiles.cpp"
#include "parse/parser.cpp"

int main(int argc, const char **args) {
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
