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

  // e.g /profiles/pki/serial/serial
  
  // Load next serial
  str serial_path = profile.source + SLASH + "pki" + SLASH + "serial" + SLASH + "serial";
  if(!fs::exists(serial_path)){
    seterror("serial file for profile '{}' not found\n", profile.name);
    return F_NOEXIST;
  }
  std::ifstream file(serial_path);
  if(!file.is_open()){
    seterror("couldn't open file '{}'\n",serial_path);
    return F_NOOPEN;
  }
  str serial;
  file >> entity.serial;

  // override default build params with user arguments
  for (int i = 0; i < opts.size(); ++i) {
    strview opt = opts[i];
    if(opt == "-keysize" || opt == "--keysize") {
      config.set(CONFIG_PKI,"key","size",std::move(opts[++i]));
    } else if (opt == "-keyformat") {
      config.set(CONFIG_PKI,"key","creation_format",std::move(opts[++i]));
    } else if (opt == "-outformat") {
      config.set(CONFIG_PKI,"csr","creation_format",std::move(opts[++i]));
    } else if (opt == "\0") {
      continue;
    } else if(opt == "-cn" || opt == "--common-name"){
      entity.subject.cn = std::move(opts[++i]);
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      entity.serial = std::move(opts[++i]);
    } else if(opt == "-loc" || opt == "--location"){
      entity.serial = std::move(opts[++i]);
    } else if(opt == "-co" || opt == "--country"){
      // todo - ensure its a 2 char
      entity.subject.country = std::move(opts[++i]);
    }else if(opt == "-org" || opt == "--organisation"){
      entity.subject.country = std::move(opts[++i]);
    }else if(opt == "-st" || opt == "--state"){
      entity.subject.state = std::move(opts[++i]);
    }
    else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  
  return actions::build(profile,config,entity);
}
