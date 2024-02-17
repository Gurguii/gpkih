#include "actions.hpp"
using namespace gpkih;

int actions::remove(std::vector<str> &profiles_to_remove, int remove_all) {
  if (remove_all) {
    return db::profiles::remove_all();
  }
  return db::profiles::remove(profiles_to_remove);
}
