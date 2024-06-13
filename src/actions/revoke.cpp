#include "actions.hpp"

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

  std::vector<std::string_view> revoked_cns{};
  std::string gopenssl_path = profile::gopenssl(profile);

  for (std::string &cn : common_names) {
    Entity *entity = nullptr;
    std::string selected_reason{};
    std::string cert_path = base_dir + SLASH + cn + "-crt.pem";
    
    if(eman.exists(cn, entity) == false){
      PWARN("entity '{}' doesn't exist\n", cn);
      continue;
    }

    for (int i = 0; i < reasons.size(); ++i) {
      fmt::print(" {} - {}\n",i,reasons[i].first);
    }

    int choice = 0;
    auto ans = PROMPT("choice: ");
    choice = strtol(&ans[0], nullptr, 10);

    if (choice < reasons.size()) {
      selected_reason = reasons[choice].second;
    }
    
    std::string command =
        fmt::format("openssl ca -config {} -revoke {} -crl_reason {}",
                    gopenssl_path, cert_path, selected_reason);

    if (system(command.c_str())) {
      PERROR("command '{}' failed\n", command);
      return GPKIH_FAIL;
    }
    
    entity->status = ES_REVOKED;
    revoked_cns.emplace_back(cn);
  }

  if(revoked_cns.empty()){
    PINFO("No entities revoked\n");
    return GPKIH_OK;
  }

  // Sync to update Entities' statuses
  eman.sync();

  std::stringstream ss{};
  for(const auto &view : revoked_cns){
    ss << view << ",";
  }
  std::string s{ss.str()};
  s.erase(s.end()-1, s.end());

  PSUCCESS("Revoked entities: {}\n", s);
  ADD_LOG(L_INFO,fmt::format("profile:{} action:revoke entities:{}",profile.name,s));

  /* Extra questions */
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  if(prompt){
    auto ans = PROMPT("Generate new crl?", "[y/n:]", true);
    if (ans == "y" || ans == "yes") {
      return actions::gencrl(profile);
    }
  }
  
  return GPKIH_OK;
};