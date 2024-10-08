#include "AAdd.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../libs/utils/utils.hpp"

#include "../../profiles/structs.hpp"

#include "../../entities/structs.hpp"
#include "../../entities/conv.hpp"
#include "../../entities/subj_utils.hpp"

#include "../../gpkih.hpp"

#include "../../config/Config.hpp"

#include "../../db/entities.hpp"
#include "../../db/profiles.hpp"

#include "add.hpp"

using namespace gpkih;

int AAdd::exec(std::vector<std::string> &args) const {
  DEBUG(1, "parsers::build()");

  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (args.empty()) {
    PERROR("Profile must be given\n");
    PHINT("Try 'gpkih help add' for extra help\n");
    return 0;
  }

  std::string_view profilename = args[0];
  
  Profile *profile; // profile to build entity for
  Entity entity{};  // entity info
  entity.meta.status = ES_NONE;

  profile = db::profiles::get(profilename);

  if(profile == nullptr){
    PERROR("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }
  
  args.erase(args.begin());

  /* Profile configuration + Entity manager */
  ProfileConfig config(*profile);
  EntityManager eman(profile->name);
  CONFIG_FILE succesfullyLoaded = config.loadedFiles();

  if( !(succesfullyLoaded & CFILE_PKI) ){
    PWARN("Couldn't load profile PKI configuration file\n");;
    return GPKIH_FAIL;
  }

  // Load next serial
  entity::loadSerial(*profile, entity);
  
  /* BEG - parse args */
  for (int i = 0; i < args.size(); ++i) {
    std::string_view opt = args[i];
    if(opt == "-algo" || opt == "--algorithm"){
      // check its a valid algorithm
      config.set(CFILE_PKI,"key","algorithm",std::move(args[++i]));
    } else if(opt == "-keysize" || opt == "--keysize") {
      config.set(CFILE_PKI,"key","size",std::move(args[++i]));
    } else if(opt == "-cn" || opt == "--common-name"){
      CALLOCATE(entity.subject.cn, reinterpret_cast<size_t*>(&entity.subject.meta.cnlen), args[++i]);
      if(eman.exists(entity.subject.cn)){
        PWARN("Entity '{}' already exists in profile '{}'\n", entity.subject.cn, profile->name);
        return GPKIH_FAIL;
      }
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      size_t serial = std::move(stoi(args[++i]));
      if(eman.exists(serial)){
        PWARN("Entity with serial '{}' already exists\n");
      }else{
        entity.meta.serial = serial;  
      }
    } else if(opt == "-loc" || opt == "--location"){
      CALLOCATE(entity.subject.location, reinterpret_cast<size_t*>(&entity.subject.meta.locationlen), args[++i]);
    } else if(opt == "-co" || opt == "--country"){
      if(gurgui::utils::str::glength(args[++i].c_str()) == 2){
        memcpy(const_cast<char*>(entity.subject.country),args[i].c_str(),2); 
      }else{
        PWARN("Country must be a 2 letter code, e.g ES,EN,DE,FR ... omitting user value '{}'\n",args[i]);
      }
    }else if(opt == "-org" || opt == "--organisation"){
      CALLOCATE(entity.subject.organisation,reinterpret_cast<size_t*>(&entity.subject.meta.organisationlen),args[++i]);
    }else if(opt == "-st" || opt == "--state"){
      CALLOCATE(entity.subject.state, reinterpret_cast<size_t*>(&entity.subject.meta.statelen), args[++i]);
    }else if(opt == "-email" || opt == "--email" || opt == "-mail" || opt == "--mail  "){
      CALLOCATE(entity.subject.email, reinterpret_cast<size_t*>(&entity.subject.email), args[++i]);
    }else if(opt == "-pfx" || opt == "--pfx"){
      config.set(CFILE_PKI, "output", "create_pfx","yes");
    }else if(opt == "-inline" || opt == "--inline"){
      config.set(CFILE_PKI, "output", "create_inline", "yes");
    }else if(opt == "-days" || opt == "--days"){
      int d = std::strtol(&args[++i][0],nullptr,10);
      if(d <= 0){
        PWARN("Days must be a positive integer, ignoring given value {:+d}\n", d);
        continue;
      }
      config.set(CFILE_PKI, "crt", "days", args[i]);
    }else if(opt == "\0"){
      continue;
    }else if(opt == "-t" || opt == "--type"){
      entity.meta.type = entity::conversion::toEnum<ENTITY_TYPE>(args[++i]); 
      if(entity.meta.type == ET_NONE){
        PERROR("Unknown entity type '{}' valid types: ca,cl,sv,client,server\n", args[i]);
        return GPKIH_FAIL;
      }
    }else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }
  /* END - parse args */

  /* BEG - Checks */
  if((entity.meta.type & ET_CL || entity.meta.type & ET_SV) && profile->meta.caCreated == false)
  {
    PWARN("CA must be created before creating server/client entities\n");
    PHINT("./gpkih new {} ca -cn {}_CA\n",profile->name, profile->name);
    return GPKIH_OK;
  }else if(entity.meta.type == ET_NONE){
    PERROR("Missing mandatory option -t | --type <ca|cl|sv|client|server>\n");
    return GPKIH_FAIL;
  }

  if(entity.subject.cn == nullptr){
    // User didn't give common_name (mandatory to build a certificate) with cli args
    auto sub = config.default_subject();  
    subject::promptForSubject(profile->name, entity.subject, sub, eman);
  }
  /* END - Checks */

  int rcode = GPKIH_FAIL;
  
  auto pkiconf = config.get(CFILE_PKI);

  /* Generic configuration retrieval */
  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  /* Profile configuration retrieval */
  std::string_view keySize = pkiconf["key"]["size"];
  std::string_view keyAlgo = pkiconf["key"]["algorithm"];
  std::string_view days    = pkiconf["crt"]["days"];

  if(entity.meta.type & ET_CA){
    
    if(entity::setCAPaths(*profile, entity) != GPKIH_OK){
      PERROR("smth failed on setCAPaths()\n");
      return GPKIH_FAIL;
    };

    rcode = actions::add::addCA(*profile,config,entity,eman,days,keyAlgo,keySize, autoanswer, prompt);
  
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
      
      if(actions::add::addCA(*profile, config, newCA, eman, days, keyAlgo, keySize, autoanswer, prompt) != GPKIH_OK){
        return GPKIH_FAIL;
      }
    }

    if(entity::setPaths(*profile,entity) != GPKIH_OK){
      PERROR("smth failed on setPaths()\n");
      return GPKIH_FAIL;
    }

    rcode = actions::add::add(*profile,config,entity,eman,days,keyAlgo,keySize, autoanswer, prompt);
  }

  if(rcode != GPKIH_OK){
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
};