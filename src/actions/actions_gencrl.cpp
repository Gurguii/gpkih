#include "actions.hpp"

using namespace gpki;
int actions::gencrl(Profile *profile, subopts::gencrl *params){
  std::string command = "openssl ca" 
  " -config " + profile->gopenssl() +
  " -gencrl" +
  " -out " + profile->dir_crl() + SLASH + "current.pem";
  if(system(command.c_str())){
    std::cout << "[FAIL] - '%s'\n";
    return -1;
  }
  return 0;
}
