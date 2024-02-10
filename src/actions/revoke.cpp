#include "actions.hpp"

static inline std::vector<std::pair<std::string,std::string>> crl_reasons(){
  return { \
  {"entity key got compromised","keyCompromised"}, \
  {"ca private key got compromised","CACompromise"}, \
  {"entity affiliation has changed","affiliationChanged"}, \
  {"certificate has been replaced with a new one","superseded"}, \
  {"entity operation ceased","cessationOfOperation"}, \
  {"certificate is on hold","certificateHold"} \
  };
}
using namespace gpki; 
int actions::revoke(subopts::revoke &params){
  Profile &profile = params.profile;
  auto reasons = crl_reasons();
  for(str cn : params.common_name){
  Entity entity;

  if(db::entities::load(profile.name, cn, entity)){
    PERROR("entity with common name '{}' doesn't exist\n",cn);
    return -1;
  }
  
  for(int i = 0; i < reasons.size(); ++i){
    std::cout << "  " << i << ".- " << reasons[i].first << std::endl;
  }

  int choice = 0;
  std::string ans;
  PROMPT("choice: ");
  std::getline(std::cin,ans);
  choice = strtol(&ans[0],nullptr,10);
  if(choice < reasons.size()){
    params.reason = reasons[choice].second;
  }

  std::string command = "openssl ca"
  " -config " + profile.gopenssl() +
  " -revoke " + entity.cert_path +  
  " -crl_reason " + params.reason;
  
  if(system(command.c_str())){
    PERROR("command '{}' failed\n", command);
    return -1;
  }
  
  PROMPT("Generate new crl?","[y/n:]");
  ans.assign("");
  std::getline(std::cin,ans);
  if(ans == "y" || ans == "Y"){
    subopts::gencrl params;
    params.profile = std::move(profile);
    actions::gencrl(params);
  }
  }

  return 0;
};
