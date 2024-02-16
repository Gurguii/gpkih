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
  // Check if user wants to set|get
  if(opts.empty()){
    PERROR("subaction set|get must be specified\n");
    PHINT("try gpki help config\n");
    return -1;
  } 
  str subaction = opts[0];
  opts.erase(opts.begin());
  if(subaction == "set"){
    std::cout << "calling gpki config SET\n";
  }else if(subaction == "get"){
    // TODO - add get action into actions/config.cpp
    std::cout << "calling gpki config GET\n";
  }else{
    PERROR("subaction '{}' doesn't exist\n", subaction);
    return -1;
  }

  return 0;
}