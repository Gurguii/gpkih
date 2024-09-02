#include "ARevoke.hpp"
#include "../../libs/printing/printing.hpp"

#include "../../profiles/structs.hpp"
#include "../../profiles/profiles.hpp"

#include "../../entities/structs.hpp"

#include "../../db/entities.hpp"
#include "../../db/profiles.hpp"

#include "../../config/Config.hpp"
//#include <sstream>
#include "../../libs/logger/enums.hpp"

#include "../actions.hpp"

#include "../../gpkih.hpp"

using namespace gpkih;

static inline std::vector<std::pair<std::string, std::string>> crl_reasons() {
  return {{"entity key got compromised", "keyCompromised"},
          {"ca private key got compromised", "CACompromise"},
          {"entity affiliation has changed", "affiliationChanged"},
          {"certificate has been replaced with a new one", "superseded"},
          {"entity operation ceased", "cessationOfOperation"},
          {"certificate is on hold", "certificateHold"}};
}

int revoke(Profile &profile, std::vector<std::string> &common_names, std::vector<std::string> &serials) {
  auto reasons = crl_reasons();
  std::string base_dir = profile::crtDir(profile);
  EntityManager eman(profile.name);

  std::vector<std::string_view> revoked_cns{};
  std::string gopenssl_path = profile::gopensslPath(profile);

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
    
    entity->meta.status = ES_REVOKED;
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
  ADD_LOG(LL_INFO,fmt::format("profile:{} action:revoke entities:{}",profile.name,s));

  /* Extra questions */
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  if(prompt){
    auto ans = PROMPT("Generate new crl?", "[y/n]", true);
    if (ans == "y" || ans == "yes") {
      std::vector<std::string> _tmpargs{profile.name};
      actions::GetAction("gencrl")->exec(_tmpargs);
    	return GPKIH_OK;
    }
    PINFO("Not generating...\n");
  }
  
  return GPKIH_OK;
};

int ARevoke::exec(std::vector<std::string> &args)const {
	/* BEG - Parse args */
	  DEBUG(1, "parsers::revoke()");

  if (args.size() == 0) {
    PERROR("profile name must be given\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }
  
  std::string_view profilename = args[0];
  Profile profile;

  std::vector<std::string> cnToRevoke;
  std::vector<std::string> serialsToRevoke;

  if (db::profiles::load(profilename, profile)) {
    PERROR("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  };
  
  args.erase(args.begin());
  args.push_back("\0");

  if (args.size() == 1) {
    PERROR("Missing serial | common_name\n");
    PHINT("Try gpki help revoke\n");
    return GPKIH_FAIL;
  }

  /* Revoke subopts */
  for(int i = 0; i < args.size(); ++i){
    std::string_view opt = args[i];
    if(opt == "-cn" || opt == "--common-name"){
      std::string cn;
      std::stringstream ss(args[++i]);
      while(getline(ss,cn,',')){
        cnToRevoke.emplace_back(cn);
      }  
    }else if(opt == "-s" || opt == "--serial"){
      std::string serial;
      std::stringstream ss(args[++i]);
      while(getline(ss,serial,',')){
        serialsToRevoke.emplace_back(serial);
      }
    }
  }

  if(cnToRevoke.empty()){
    PERROR("No common names provided, did you miss -cn | --common-name?\n");
    return GPKIH_FAIL;
  }

  args.erase(args.begin(), args.begin() + 2);

  return revoke(profile, cnToRevoke, serialsToRevoke);
  /* END - Parse args */
}