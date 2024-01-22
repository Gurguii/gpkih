#include "actions.hpp"

#define crl_reasons std::vector<std::pair<std::string,std::string>>{ \
  {"entity key got compromised","keyCompromised"}, \
  {"ca private key got compromised","CACompromise"}, \
  {"entity affiliation has changed","affiliationChanged"}, \
  {"certificate has been replaced with a new one","superseded"}, \
  {"entity operation ceased","cessationOfOperation"}, \
  {"certificate is on hold","certificateHold"} \
 }
using namespace gpki; 
int actions::revoke(Profile *profile, subopts::revoke *params){
  Entity entity;

  if(db::entities::load(profile, &entity, params->common_name)){
    std::cout << "[error] - entity with common name '" << params->common_name << "' doesn't exist\n";
    return -1;
  }
  auto reasons = crl_reasons;
  for(int i = 0; i < reasons.size(); ++i){
    std::cout << i << ". " << reasons[i].first << "\n";
  }
  int choice = 0;
  std::string ans;
  std::cout << "choice: ";
  std::getline(std::cin,ans);
  choice = strtol(&ans[0],nullptr,10);
  if(choice < reasons.size()){
    params->reason = crl_reasons[choice].second;
  }
  std::string command = "openssl ca"
  " -config " + profile->gopenssl() +
  " -revoke " + entity.cert_path +  
  " -crl_reason " + params->reason;
  
  if(system(command.c_str())){
    std::cout << "command '" + command + "' failed\n";
    return -1;
  }
  
  std::cout << "Generate new crl? y/n:";
  ans.assign("");
  std::getline(std::cin,ans);
  if(ans == "y" || ans == "Y"){
    subopts::gencrl params;
    actions::gencrl(profile,&params);
  }
  return 0;
};
