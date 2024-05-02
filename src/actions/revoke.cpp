#include "actions.hpp"
#include <iostream> // std::cin

static inline std::vector<std::pair<std::string, std::string>> crl_reasons() {
  return {{"entity key got compromised", "keyCompromised"},
          {"ca private key got compromised", "CACompromise"},
          {"entity affiliation has changed", "affiliationChanged"},
          {"certificate has been replaced with a new one", "superseded"},
          {"entity operation ceased", "cessationOfOperation"},
          {"certificate is on hold", "certificateHold"}};
}

using namespace gpkih;
int actions::revoke(Profile &profile, std::vector<std::string> &common_names, std::vector<std::string> &serials) {
  auto reasons = crl_reasons();
  std::string base_dir = profile::dir_crt(profile);
  EntityManager eman(profile.name);

  for (std::string &cn : common_names) {
    Entity entity;
    std::string selected_reason{};
    std::string cert_path = base_dir + SLASH + cn + "-crt.PEM";

    // CHANGETHIS
    std::string pname = profile.name;
    
    if(eman.exists(cn) == false){
      PWARN("entity '{}' doesn't exist\n", cn);
      continue;
    }

    for (int i = 0; i < reasons.size(); ++i) {
      fmt::print("  {}.- \n",reasons[i].first);
    }

    int choice = 0;
    auto ans = PROMPT("choice: ");
    choice = strtol(&ans[0], nullptr, 10);

    if (choice < reasons.size()) {
      selected_reason = reasons[choice].second;
    }
    
    std::string command =
        fmt::format("openssl ca -config {} -revoke {} -crl_reason {}",
                    profile::gopenssl(profile), cert_path, selected_reason);

    if (system(command.c_str())) {
      seterror("command '{}' failed\n", command);
      return GPKIH_FAIL;
    }

    ans = PROMPT("Generate new crl?", "[y/n:]", true);
    if (ans == "y" || ans == "yes") {
      actions::gencrl(profile);
    }
  }

  return 0;
};
