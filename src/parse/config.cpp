#include "parser.hpp"

using namespace gpki;

/* gpki config 
 * ./gpki config <profile> get <prop1> <prop2> ... <propN> 
 * ./gpki config <profile> set <prop1>=<nval1> ... <propN>=<nvalN> 
 */

// Checks if the profile exists, if it does it populates the
// Profile structure and removes the first element from the vector
// which should be the profile name, else it sets the last error and returns
int check_and_populate_profile(strview profilename, Profile &buffer, std::vector<str> &opts){
  if(db::profiles::load(profilename,buffer)){
    // profile does not exist
    seterror("Profile '{}' doesn't exist\n", profilename);
    return -1;
  }
  // delete profile name from vector
  opts.erase(opts.begin());
  return 0;
};

int parse_get(std::vector<str> &opts, subopts::get &params)
{
  str section, prop; // valid scopes -> common client server
  sstream ss;
  std::vector<str> nonexist_props;

  for (str &properties : opts) {
    ss.clear();
    ss << properties;
    getline(ss, section, '.');
    getline(ss,prop);
    if (section == "client" || section == "cl") {
      if (prop.empty() && params.cl_properties.empty()) {
        for (const auto &kv : GpkihConfig::client) {
          params.cl_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::client_prop_exists(prop)) {
        params.cl_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    } else if (section == "server" || section == "sv") {
      if (prop.empty() && params.sv_properties.empty()) {
        for (const auto &kv : GpkihConfig::server) {
          params.sv_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::server_prop_exists(prop)) {
        params.sv_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    } else if (section == "common" || section == "com") {
      if (prop.empty() && params.common_properties.empty()) {
        for (const auto &kv : GpkihConfig::server) {
          params.common_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::common_prop_exists(prop)) {
        params.common_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    } else if(section == "pki"){
      
    }else{
      PERROR("unknown section '{}'\n", section);
    }
  }

  if (!nonexist_props.empty()) {
    PWARN("unexistant properties: ");
    for (auto &property : nonexist_props) {
      PRINTF(fg(COLOR::medium_violet_red) | EMPHASIS::italic, "{} ", property);
    }
    std::cout << "\n";
  }

  return 0;
}

int parse_set(std::vector<str> &opts, subopts::set &params){
  str section,prop,nval;
  sstream ss;
  std::vector<str>nonexist_props;
  map<str,std::pair<str,str>> changed_properties{};
  for(str &properties : opts){
      ss.clear();;
      ss << properties;
      getline(ss,section,'.');
      getline(ss,prop,'=');
      getline(ss,nval);
      if(section == "client" || section == "cl"){
        if(GpkihConfig::client_prop_exists(prop)){
          strview oldval = GpkihConfig::client[prop]; 
          GpkihConfig::set_client_prop(prop,nval);
          changed_properties[prop] = std::pair{oldval,nval};
        }
      }else if(section == "server" || section == "sv"){
        if(GpkihConfig::server_prop_exists(prop)){
          strview oldval = GpkihConfig::server[prop];
          GpkihConfig::set_server_prop(prop,nval);
          changed_properties[prop] = std::pair{oldval,nval};
        }
      }else if(section == "common"){
        if(GpkihConfig::common_prop_exists(prop)){
          strview oldval = GpkihConfig::common[prop];
          GpkihConfig::set_common_prop(prop,nval);
          changed_properties[prop] = std::pair{oldval,nval};
        }
      }else if(section == "pki"){
        if(GpkihConfig::pki_prop_exists(prop)){
          strview oldval = GpkihConfig::pki[prop];
          GpkihConfig::set_pki_prop(prop,nval);  
          changed_properties[prop] = std::pair{oldval,nval};
        }
      }else{
        PERROR("section '{}' doesn't exist\n",section);
        continue;
      }
  }
  if(GpkihConfig::sync()){
    return -1;
  }
  if(!changed_properties.empty()){
    fmt::print(fg(COLOR::lawn_green) | EMPHASIS::italic | EMPHASIS::underline,"Changed properties\n");
    for(auto &kv : changed_properties){
      fmt::print("{}: {} -> {}\n", kv.first, kv.second.first, kv.second.second);
    }
  }
  return 0;
}

// Does generic checks and calls appropiate 
// sub_sub_parser
int parsers::config(std::vector<str> opts){
  if(opts.empty()){
    PERROR("profile name must be given\n");
    PHINT("try gpki help config\n");
    return -1;
  }
  Profile profile;
  if(check_and_populate_profile(opts[0],profile,opts)){
    return -1;
  }
  // Profile exists, load configuration 
  GpkihConfig::load(profile);
  // Check if user wants to set|get
  if(opts.empty()){
    PERROR("subaction set|get must be specified\n");
    PHINT("try gpki help config\n");
    return -1;
  } 
  str subaction = opts[0];
  opts.erase(opts.begin());
  if(subaction == "set"){
    auto params = subopts::set{.profile{std::move(profile)}};
    parse_set(opts,params);
    return actions::set(params);
  }else if(subaction == "get"){
    auto params = subopts::get{.profile{std::move(profile)}};
    // TODO - add get action into actions/config.cpp
    return parse_get(opts,params);
  }else{
    PERROR("subaction '{}' doesn't exist\n", subaction);
    return -1;
  }

  return 0;
}
