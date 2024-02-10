#include "actions.hpp"
using namespace gpki;

int actions::remove(subopts::remove &params) {
  if (params.all) {
    return db::profiles::remove_all(params.autoanswer_yes);
  }
  return db::profiles::remove(params.profiles,params.autoanswer_yes);
}
