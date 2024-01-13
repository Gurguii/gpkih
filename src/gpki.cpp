#include "db/entities.cpp"
#include "db/profiles.cpp"
#include "help/generic.cpp"
#include "parse/parser.cpp"
#include "modes/build.cpp"
#include "modes/entity.cpp"
#include "modes/profile.cpp"
int main(int argc, const char **args) {
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
