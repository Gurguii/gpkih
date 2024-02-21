#include "actions.hpp"

using namespace gpkih;
int actions::gencrl(Profile &profile) {
  str command =
      fmt::format("openssl ca -config {} -gencrl -out {}{}current.pem",
                  profile.gopenssl(), profile.dir_crl(), SLASH);
  if (system(command.c_str())) {
    seterror("openssl gencrl command failed - '%s'\n");
    return GPKIH_FAIL;
  }
  return GPKIH_OK;
}

