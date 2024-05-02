#include "actions.hpp"
using namespace gpkih;

int actions::remove(std::vector<str> &profiles_to_remove, int remove_all) {
  if (remove_all) {
    size_t deleted = db::profiles::remove_all();
    PINFO("Deleted [{}] profiles\n", deleted); 
    return deleted >= 0 ? GPKIH_OK : GPKIH_FAIL;
  }
  size_t removed = 0;
  for (auto p : profiles_to_remove) {
      if (db::profiles::remove(p) == GPKIH_FAIL) {
          PWARN("couldn't remove profile {}\n", p);
      }
      else {
          ++removed;
      };
  }
  return removed;
}