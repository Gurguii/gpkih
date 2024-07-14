#include "../parser.hpp"
#include "../../db/profiles.hpp"
#include "../../entities/subj_utils.hpp"
#include "../../libs/utils/utils.hpp"
#include "../../entities/conv.hpp"
#include "../../config/Config.hpp"

#include <cstdlib>

using namespace gpkih;

int parsers::build(std::vector<std::string> &opts) {
  DEBUG(1, "parsers::build()");

  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  std::string_view profilename = opts[0];
  
  Profile *profile; // profile to build entity for
  Entity entity{};  // entity info
  entity.meta.status = ES_NONE;

  profile = db::profiles::get(profilename);
  if(profile == nullptr){
    PERROR("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }
  
  if(opts.size() == 1){
    PERROR("Entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  std::string_view eTypeStr = opts[1];

  if(eTypeStr == "cl" || eTypeStr == "client"){
    entity.meta.type = ET_CL;
  }else if(eTypeStr == "sv" || eTypeStr == "server"){
    entity.meta.type = ET_SV;
  }else if(eTypeStr == "ca"){
    entity.meta.type = ET_CA;
  }else{
    PERROR("Invalid entity type '{}', valid entities: cl|client|sv|server|ca\n",eTypeStr);
    return GPKIH_FAIL;
  }

  opts.erase(opts.begin(),opts.begin()+2);
  
  /* Profile configuration + Entity manager */
  ProfileConfig config(*profile);
  EntityManager eman(profile->name);
  CONFIG_FILE succesfullyLoaded = config.loadedFiles();

  if( (succesfullyLoaded & CFILE_PKI) == false){
    PWARN("Couldn't load profile PKI configuration file\n");;
    return GPKIH_FAIL;
  }

  if((entity.meta.type & ET_CL || entity.meta.type & ET_SV) && profile->meta.caCreated == false)
  {
    PWARN("CA must be created before creating server/client entities\n");
    PHINT("./gpkih build {} ca -cn MyCA\n",profile->name);
    return GPKIH_OK;
  }

  // Load next serial
  entity::loadSerial(*profile, entity);
  
  // override default build params with user arguments
  for (int i = 0; i < opts.size(); ++i) {
    std::string_view opt = opts[i];

    if(opt == "-algo" || opt == "--algorithm"){
      // check its a valid algorithm
      config.set(CFILE_PKI,"key","algorithm",std::move(opts[++i]));
    } else if(opt == "-keysize" || opt == "--keysize") {
      config.set(CFILE_PKI,"key","size",std::move(opts[++i]));
    } else if(opt == "-cn" || opt == "--common-name"){
      CALLOCATE(entity.subject.cn, reinterpret_cast<size_t*>(&entity.subject.meta.cnlen), opts[++i]);
      if(eman.exists(entity.subject.cn)){
        PWARN("Entity '{}' already exists in profile '{}'\n", entity.subject.cn, profile->name);
        return GPKIH_FAIL;
      }
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      size_t serial = std::move(stoi(opts[++i]));
      if(eman.exists(serial)){
        PWARN("Entity with serial '{}' already exists\n");
      }else{
        entity.meta.serial = serial;  
      }
    } else if(opt == "-loc" || opt == "--location"){
      CALLOCATE(entity.subject.location, reinterpret_cast<size_t*>(&entity.subject.meta.locationlen), opts[++i]);
    } else if(opt == "-co" || opt == "--country"){
      if(gurgui::utils::str::glength(opts[++i].c_str()) == 2){
        memcpy(const_cast<char*>(entity.subject.country),opts[i].c_str(),2); 
      }else{
        PWARN("Country must be a 2 letter code, e.g ES,EN,DE,FR ... omitting user value '{}'\n",opts[i]);
      }
    }else if(opt == "-org" || opt == "--organisation"){
      CALLOCATE(entity.subject.organisation,reinterpret_cast<size_t*>(&entity.subject.meta.organisationlen),opts[++i]);
    }else if(opt == "-st" || opt == "--state"){
      CALLOCATE(entity.subject.state, reinterpret_cast<size_t*>(&entity.subject.meta.statelen), opts[++i]);
    }else if(opt == "-email" || opt == "--email"){
      CALLOCATE(entity.subject.email, reinterpret_cast<size_t*>(&entity.subject.email), opts[++i]);
    }else if(opt == "-pfx" || opt == "--pfx"){
      config.set(CFILE_PKI, "output", "create_pfx","yes");
    }else if(opt == "-inline" || opt == "--inline"){
      config.set(CFILE_PKI, "output", "create_inline", "yes");
    }else if(opt == "-days" || opt == "--days"){
      int d = std::strtol(&opts[++i][0],nullptr,10);
      if(d <= 0){
        PWARN("Days must be a positive integer, ignoring given value {:+d}\n", d);
        continue;
      }
      config.set(CFILE_PKI, "crt", "days", opts[i]);
    }else if(opt == "\0"){
      continue;
    }else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }

  if(entity.subject.cn == nullptr){
    // User didn't give common_name (mandatory to build a certificate) with cli opts
    auto sub = config.default_subject();  
    subject::promptForSubject(profile->name, entity.subject, sub, eman);
  }

  int rcode = GPKIH_FAIL;
  
  auto pkiconf = config.get(CFILE_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  std::string_view keySize = pkiconf["key"]["size"];
  std::string_view keyAlgo = pkiconf["key"]["algorithm"];
  std::string_view days = pkiconf["crt"]["days"];

  if(entity.meta.type & ET_CA){
    
    if(entity::setCAPaths(*profile, entity) != GPKIH_OK){
      PERROR("smth failed on setCAPaths()\n");
      return GPKIH_FAIL;
    };

    rcode = actions::build_ca(*profile,config,entity,eman,days,keyAlgo,keySize);
  
  }else if(entity.meta.type & ET_CL || entity.meta.type & ET_SV){
    
    if(profile->meta.caCreated == false){
      auto ans = PROMPT("Certificate authority (CA) hasn't been created yet, create?","[y/n]",true);
      
      if(ans != "y" && ans != "yes"){
        PERROR("Can't create server/client certificates without CA\n");
        return GPKIH_FAIL;  
      }

      auto sub = config.default_subject();
      Entity newCA{};
      subject::promptForSubject(profile->name, newCA.subject, sub, eman);
      
      if(actions::build_ca(*profile, config, newCA, eman, days, keyAlgo, keySize) == GPKIH_FAIL){
        return GPKIH_FAIL;
      }
    }

    if(entity::setPaths(*profile,entity) != GPKIH_OK){
      PERROR("smth failed on setPaths()\n");
      return GPKIH_FAIL;
    }

    rcode = actions::build(*profile,config,entity,eman,days,keyAlgo,keySize);
  }

  if(rcode == GPKIH_FAIL){
    return rcode;
  }

  entity.meta.expirationDate = entity.meta.creationDate + std::chrono::seconds(3600*24*std::stoull(days.data(),nullptr,10));
  entity.meta.status = ES_ACTIVE;

  eman.add(entity);
  db::profiles::sync();
  eman.sync();

  PSUCCESS("Entity '{}' created\n", entity.subject.cn);
  ADD_LOG(LL_INFO,fmt::format("profile:{} action:build serial:{} cn:{} type:{}",profile->name, entity.meta.serial, entity.subject.cn, entity::conversion::toString(entity.meta.type)));
  
  return GPKIH_OK;
}