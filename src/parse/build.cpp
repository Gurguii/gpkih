#include "parser.hpp"

using namespace gpkih;
int parsers::build(std::vector<std::string> opts) {
  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  strview profilename = opts[0];
  
  Profile profile; // profile to build entity for
  Entity entity;  // entity info

  if(db::profiles::load(profilename, profile)){
    return GPKIH_FAIL;
  };

  if(opts.size() == 1){
    PERROR("entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  strview etype = opts[1];
  if(etype == "ca"){
    entity.type = ET_CA;
  }else if(etype == "client" || etype == "cl"){
    entity.type = ET_CL;
  }else if(etype == "server" || etype == "sv"){
    entity.type = ET_SV;
  }else{
    PERROR("unknown entity type '{}'\n", etype);
    return GPKIH_FAIL;
  }

  opts.erase(opts.begin(),opts.begin()+2);

  ProfileConfig config(profile);
  if(!config.succesfully_loaded){
    return GPKIH_FAIL;
  }

  // override default build params with user arguments and set
  for (int i = 0; i < opts.size(); ++i) {
    strview opt = opts[i];
    if(opt == "-keysize" || opt == "--keysize") {
      config.set(CONFIG_PKI,"key","size",opts[++i]);
    } else if (opt == "-keyformat") {
      config.set(CONFIG_PKI,"key","creation_format",opts[++i]);
    } else if (opt == "-outformat") {
      config.set(CONFIG_PKI,"csr","creation_format",opts[++i]);
    } else if (opt == "\0") {
      continue;
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  
  return actions::build(profile,config,entity);
}
