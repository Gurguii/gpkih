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
int actions::revoke(Profile &profile, std::vector<str> &common_names, std::vector<str> &serials) {
  auto reasons = crl_reasons();
  str base_dir = profile.dir_crt();
  
  for (str &cn : common_names) {
    Entity entity;
    str selected_reason{};
    str cert_path = base_dir + SLASH + cn + "-crt.pem";
    
    if (db::entities::load(profile.name, cn, entity)) {
      PERROR("entity with common name '{}' doesn't exist\n", cn);
      return -1;
    }

    for (int i = 0; i < reasons.size(); ++i) {
      fmt::print("  {}.- \n",reasons[i].first);
    }

    int choice = 0;
    std::string ans;
    PROMPT("choice: ");
    std::getline(std::cin, ans);
    choice = strtol(&ans[0], nullptr, 10);
    if (choice < reasons.size()) {
      selected_reason = reasons[choice].second;
    }
    str command =
        fmt::format("openssl ca -config {} -revoke {} -crl_reason {}",
                    profile.gopenssl(), cert_path, selected_reason);

    if (system(command.c_str())) {
      seterror("command '{}' failed\n", command);
      return GPKIH_FAIL;
    }

    PROMPT("Generate new crl?", "[y/n:]");
    ans.assign("");
    std::getline(std::cin, ans);
    if (ans == "y" || ans == "Y") {
      actions::gencrl(profile);
    }
  }

  return 0;
};
