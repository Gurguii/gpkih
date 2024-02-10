#include "actions.hpp"
using namespace gpki;

int actions::remove(subopts::remove &params) {
  if (params.all) {
    return db::profiles::remove_all();
  }
  return db::profiles::remove(params.profiles);
}
