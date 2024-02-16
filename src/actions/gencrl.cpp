#include "actions.hpp"

using namespace gpki;
int actions::gencrl(subopts::gencrl &params){
  Profile &profile = params.profile;
  str command = fmt::format("openssl ca -config {} -gencrl -out {}{}current.pem",profile.gopenssl(), profile.dir_crl(), SLASH);
  if(system(command.c_str())){
    std::cout << "[FAIL] - '%s'\n";
    return -1;
  }
  return 0;
}
