#include "parser.hpp"

using namespace gpkih;

static inline size_t __load_serial(Profile *profile, Entity &entity){
  PDEBUG(2,"__load_serial()");

  CALLOCATE(serialPath, &serialPathLen, fmt::format("{}{}pki{}serial{}serial", profile->source, SLASH, SLASH, SLASH));

  if(!fs::exists(serialPath)){
    PERROR("serial file for profile '{}' not found\n", profile->name);
    return GPKIH_FAIL;
  }

  std::ifstream file(serialPath);

  if(!file.is_open()){
    PERROR("couldn't open file '{}'\n",serialPath);
    return GPKIH_FAIL;
  }

  std::string serial{};
  file >> serial;
  file.close();

  entity.serial = std::stoull(serial,NULL,16);

  if(entity.serial <= 0){
    PERROR("didn't load serial properly\n");
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
};

using namespace gpkih;

int parsers::build(std::vector<std::string> &opts) {
  PDEBUG(1, "parsers::build()");

  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  std::string_view profilename = opts[0];
  
  Profile *profile; // profile to build entity for
  Entity entity;  // entity info

  
  profile = db::profiles::get(profilename);
  if(profile == nullptr){
    PERROR("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }
  
  if(opts.size() == 1){
    PERROR("Entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  std::string_view etype = opts[1];
  if(entity_type_map.find(etype.data()) == entity_type_map.end()){
    PERROR("Invalid entity type '{}'",etype);
    return GPKIH_FAIL;
  }
  
  entity.type = entity_type_map[etype.data()];
  opts.erase(opts.begin(),opts.begin()+2);
  
  /* Profile configuration + Entity manager */
  ProfileConfig config(*profile);
  EntityManager eman(profile->name);
  CONFIG_FILE succesfullyLoaded = config.loadedFiles();

  if( (succesfullyLoaded & CONFIG_PKI) == false){
    PWARN("Couldn't load profile PKI configuration file\n");;
    return GPKIH_FAIL;
  }

  if((entity.type & ET_CL || entity.type & ET_SV) && profile->ca_created == false)
  {
    PWARN("CA must be created before creating server/client entities\n");
    PHINT("./gpkih build {} ca -cn MyCA\n",profile->name);
    return GPKIH_OK;
  }

  // Load next serial
  __load_serial(profile, entity);
  
  // override default build params with user arguments
  for (int i = 0; i < opts.size(); ++i) {
    std::string_view opt = opts[i];

    if(opt == "-algo" || opt == "--algorithm"){
      // check its a valid algorithm
      config.set(CONFIG_PKI,"key","algorithm",std::move(opts[++i]));
    } else if(opt == "-keysize" || opt == "--keysize") {
      config.set(CONFIG_PKI,"key","size",std::move(opts[++i]));
    } else if(opt == "-cn" || opt == "--common-name"){
      CALLOCATE(entity.subject.cn, reinterpret_cast<size_t*>(&entity.subject.cnlen), opts[++i]);
      if(eman.exists(entity.subject.cn)){
        PWARN("entity '{}' already exists in profile '{}'\n", entity.subject.cn, profile->name);
        return GPKIH_FAIL;
      }
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      size_t serial = std::move(stoi(opts[++i]));
      if(eman.exists(serial)){
        PWARN("entity with serial '{}' already exists\n");
      }else{
        entity.serial = serial;  
      }
    } else if(opt == "-loc" || opt == "--location"){
      CALLOCATE(entity.subject.location, reinterpret_cast<size_t*>(&entity.subject.locationlen), opts[++i]);
    } else if(opt == "-co" || opt == "--country"){
      if(len(opts[++i].c_str()) == 2){
        memcpy(entity.subject.country,opts[i].c_str(),2); 
      }else{
        PWARN("country must be a 2 letter code, e.g ES,EN,DE,FR ... omitting user value '{}'\n",opts[i]);
      }
    }else if(opt == "-org" || opt == "--organisation"){
      CALLOCATE(entity.subject.organisation,reinterpret_cast<size_t*>(&entity.subject.organisationlen),opts[++i]);
    }else if(opt == "-st" || opt == "--state"){
      CALLOCATE(entity.subject.state, reinterpret_cast<size_t*>(&entity.subject.statelen), opts[++i]);
    }else if(opt == "-email" || opt == "--email"){
      CALLOCATE(entity.subject.email, reinterpret_cast<size_t*>(&entity.subject.email), opts[++i]);
    }else if(opt == "-pfx" || opt == "--pfx"){
      config.set(CONFIG_PKI, "output", "create_pfx","yes");
    }else if(opt == "-inline" || opt == "--inline"){
      config.set(CONFIG_PKI, "output", "create_inline", "yes");
    }else if(opt == "-days" || opt == "--days"){
      int d = std::strtol(&opts[++i][0],NULL,10);
      if(d <= 0){
        PWARN("days must be a positive integer, ignoring given value {:+d}\n", d);
        continue;
      }
      config.set(CONFIG_PKI, "crt", "days", opts[i]);
    }else if(opt == "\0"){
      continue;
    }else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }

  if(entity.subject.cn == nullptr){
    // User didn't give common_name (mandatory to build a certificate) with cli opts  
    utils::entities::promptForSubject(profile->name, entity.subject, config, eman);
  }

  int rcode = GPKIH_FAIL;
  
  auto pkiconf = config.get(CONFIG_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  std::string_view keySize = pkiconf["key"]["size"];
  std::string_view keyAlgo = pkiconf["key"]["algorithm"];
  std::string_view days = pkiconf["crt"]["days"];

  // TODO - load paths here.
  if(entity.type & ET_CA){
    
    if(utils::entities::setCAPaths(*profile, entity) != GPKIH_OK){
      PERROR("smth failed on setCAPaths()\n");
      return GPKIH_FAIL;
    };

    PINFO("key:{} crt:{}\n", entity.keyPath, entity.crtPath);
    
    rcode = actions::build_ca(*profile,config,entity,eman,days,keyAlgo,keySize);
  
  }else if(entity.type & ET_CL || entity.type & ET_SV){
    
    if(profile->ca_created == false){
      auto ans = PROMPT("Certificate authority (CA) hasn't been created yet, create?","[y/n]",true);
      
      if(ans == "y" && ans == "yes"){
        PERROR("Can't create server/client certificates without CA\n");
        return GPKIH_FAIL;  
      }

      Entity nent;
      utils::entities::promptForSubject(profile->name, nent.subject, config, eman);
      if(actions::build_ca(*profile, config, nent, eman, days, keyAlgo, keySize) == GPKIH_FAIL){
        return GPKIH_FAIL;
      }
    }

    if(utils::entities::setPaths(*profile,entity) != GPKIH_OK){
      PERROR("smth failed on setPaths()\n");
      return GPKIH_FAIL;
    }
    PINFO("key:{} csr: {} crt:{}\n", entity.keyPath, entity.csrPath, entity.crtPath);
    
    rcode = actions::build(*profile,config,entity,eman,days,keyAlgo,keySize);
  }

  if(rcode == GPKIH_FAIL){
    return rcode;
  }

  PSUCCESS("Entity '{}' created\n", entity.subject.cn);
  ADD_LOG(L_INFO, "profile:{} action:build serial:{} cn:{} type:{}",profile->name, entity.serial, entity.subject.cn, to_str(entity.type));
  
  return GPKIH_OK;
}