#include "actions.hpp"

using namespace gpkih;
int actions::gencrl(Profile &profile) {
  
  if(profile.ca_created == false){
    PWARN("Cannot generate a crl if no CA has been created\n");
    PHINT("Try './gpkih build <profile> ca -cn myCA' to create a certificate authority\n");
    return GPKIH_OK;
  }

  std::string crlPath = (profile::dir_crl_fs(profile)/"current.pem").string();

  std::string command =
      fmt::format("openssl ca -config {} -gencrl -out {}",
                  profile::gopenssl(profile), crlPath);

  if (system(command.c_str())) {
    PERROR("openssl gencrl command failed - '{}'\n", command);
    return GPKIH_FAIL;
  }

  PSUCCESS("Generated CRL - {}\n", crlPath);
  ADD_LOG(L_INFO,"profile:{} action:gencrl",profile.name);
  
  return GPKIH_OK;
}