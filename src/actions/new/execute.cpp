#include "ANew.hpp"

#include "../../gpkih.hpp"

#include "../../libs/utils/utils.hpp"

#include "../../profiles/profiles.hpp"
#include "../../profiles/structs.hpp"

#include "../../entities/structs.hpp"
#include "../../entities/conv.hpp"
#include "../../entities/subj_utils.hpp"

#include "../../libs/printing/printing.hpp"

#include "../../db/profiles.hpp"
#include "../../db/entities.hpp"

#include "../../config/ProfileConfig.hpp"
#include "../../config/Config.hpp"

#include "../../macros.hpp"

using namespace gpkih;

static int __create_outdir(fs::path &path){
  DEBUGF(2,"__create_outdir({})",path.string());

  if(path.is_relative()){
    PERROR("path to __create_outdir() must be absolute, given path: {}", path.string());
    return GPKIH_FAIL;
  }

  if(fs::exists(path) == false){
    return fs::create_directory(path) ? GPKIH_OK : GPKIH_FAIL;
  }else if(fs::is_directory(path) == false){
    fs::remove_all(path);
    return fs::create_directory(path) ? GPKIH_OK : GPKIH_FAIL;  
  }

  return GPKIH_OK;
};

static std::pair<std::string,std::string> __server_client_build_commands(Profile &profile, ConfigMap &pkiconf,
                                             Entity &entity, std::string_view days, std::string_view keyAlgo, std::string_view keySize) {
  DEBUG(2, "__server_client_build_commands()");

  Subject &subject = entity.subject;
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc

  std::string csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -noenc",
    keyAlgo,
    keySize,
    entity.csrPath,
    entity.keyPath,
    subject::opensslOneliner(subject)
  ));
  
  std::string x509_extensions_file_path = CONF_DIRPATH + "x509" + SLASH + entity::conversion::toString(entity.meta.type);
  
  std::string crt_command = std::move(fmt::format("openssl ca -config \"{}\" -in \"{}\" -out \"{}\" -extfile \"{}\" -days {}",
    profile::gopensslPath(profile),
    entity.csrPath,
    entity.crtPath,
    x509_extensions_file_path,
    days
  ));

  // Check additional opts based on config
  if(Config::get("behaviour","autoanswer") == "yes"){
    crt_command += " -batch";
  }
    
  if(Config::get("behaviour","print_generated_certificate") == "no"){
    crt_command += " -notext";
  }

  return {std::move(csr_command),std::move(crt_command)};
}

static std::string __ca_build_command(Profile &profile, ConfigMap &pkiconf, Entity &entity, std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  DEBUG(2, "__ca_build_command()");

  std::string command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc -days {}",
    profile::gopensslPath(profile),
    entity.crtPath,
    entity.keyPath,
    subject::opensslOneliner(entity.subject),
    entity.meta.serial,
    keyAlgo,
    keySize,
    days
  ));
  
  // Check for optional arguments based on configuration
  if(Config::get("behaviour","autoanswer") == "yes"){
    command += " -batch"; // doesn't ask before signing certificate or updating database
  }

  return std::move(command);
};

static int __create_inline_config(Profile &profile,ProfileConfig &config,
                                 std::vector<const Entity*> &entities,bool autoanswer, bool prompt) {
  DEBUG(2, "__create_inline_config()");
  std::string caCertPath = std::move(profile::caCertificatePath(profile));

  if (!fs::exists(caCertPath)) {
    PERROR("ca cert path '{}' doesn't exist\n", caCertPath);
    return GPKIH_FAIL;
  }

  // Load CA certificate
  size_t caCertSize = fs::file_size(caCertPath);
  char *caCertBuffer = (char*)ALLOCATE(caCertSize);

  if(caCertBuffer == nullptr){
    PERROR("couldn't load CA certificate '{}'\n", caCertPath);
    return GPKIH_FAIL;  
  }

  std::ifstream(caCertPath).read(caCertBuffer, caCertSize);

  for (auto &entity : entities) 
  {
    auto entityDir = fs::path{profile.source} / "packs" / entity->subject.cn;
    if (__create_outdir(entityDir) == GPKIH_FAIL) {
      return GPKIH_FAIL;
    };

    auto entityInlinePath = entityDir / fmt::format("inline_{}.{}",entity->subject.cn,vpnConfigExtension);

    if(config.dump_vpn_conf(entityInlinePath, entity->meta.type) == false){
      return GPKIH_FAIL;
    }

    if (!fs::exists(entity->keyPath)) {
      PERROR("Missing entity key '{}'\n", entity->subject.cn);
      return GPKIH_FAIL;
    }

    if(!fs::exists(entity->crtPath)){
      PERROR("Missing entity crt '{}'\n", entity->subject.cn);
      return GPKIH_FAIL;
    }
    
    std::ofstream file(entityInlinePath, std::ios::app);
    
    if (!file.is_open()) {
      PERROR("Couldn't open entity config file '{}'\n", entityInlinePath.string());
      return GPKIH_FAIL;
    }

    std::ifstream ekey(entity->keyPath);
    std::ifstream ecrt(entity->crtPath);

    if (!ekey.is_open()) {
      PERROR("Couldn't open entity key file '{}'\n", entity->keyPath);
      return GPKIH_FAIL;
    }

    if (!ecrt.is_open()) {
      PERROR("Couldn't open entity certificate file '{}'\n", entity->crtPath);
      return GPKIH_FAIL;
    }

    // add inline entity key
    file << "<key>" << EOL << ekey.rdbuf() << "</key>" << EOL;
    // add inline entity certificate
    file << "<crt>" << EOL << ecrt.rdbuf() << "</crt>" << EOL;
    // add inline ca certificate
    file << "<ca>" << EOL << caCertBuffer << "</ca>" << EOL;

    ecrt.close();
    ekey.close();

    FREE_MEMORY_BLOCK(caCertBuffer, caCertSize);
  }
  return GPKIH_OK;
}

int __create_pfx(Profile &profile, Entity &entity){
  DEBUG(2, "__create_pfx()");

  auto entityDir = fs::path{profile.source}/"packs"/entity.subject.cn;
  auto outpathPfx = entityDir / entity.subject.cn;

  if(__create_outdir(entityDir) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  std::string command = fmt::format("openssl pkcs12 -export -inkey {} -in {} -out {}.pfx",
    entity.keyPath,entity.crtPath,outpathPfx.string());

  if(system(command.c_str())){
    PERROR("Command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

extern int build_ca(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman,std::string_view days, std::string_view keyAlgo, std::string_view keySize)
{
	 DEBUG(1,"actions::build_ca()");

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;

  if(autoanswer == false && profile.meta.caCreated){
    auto ans = PROMPT("Profile already has CA and\ncreating a new one will cause newly created certificates not to work with older ones , continue?", "[y/n]", true, LGREEN);
    if(ans != "y" && ans != "yes"){
      return GPKIH_FAIL;
    } 
  }

  ConfigMap &pkiconf = config.get(CFILE_PKI);
  std::string command = std::move(__ca_build_command(profile,pkiconf,entity,days,keyAlgo,keySize));
  
  DEBUGF(3, "self-signed CA command - '{}'", command);
  
  if(system(command.c_str())){
    PERROR("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  entity::incrementSerial(profile, entity);
  
  // Update profile modification date + caCreated status
  profile.meta.lastModification = std::chrono::system_clock::now();
  profile.meta.caCreated = 1;

  return GPKIH_OK;
};

extern int build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize)
{
	  DEBUG(1,"actions::build()");

  ConfigMap &pkiconf = config.get(CFILE_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  const auto [req_command, crt_command] = __server_client_build_commands(profile, pkiconf, entity, days, keyAlgo, keySize);

  // Create key + csr
  DEBUGF(2,"executing key + certificate request command - {}",req_command);
  if(system(req_command.c_str())){
    // fail
    PERROR("[ REQUEST ] command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // Create crt
  DEBUGF(2,"executing certificate command - {}",crt_command);
  if(system(crt_command.c_str())){
    // fail
    PERROR("[ CERTIFICATE SIGNING ] command '{}' failed\n",crt_command);
    return GPKIH_FAIL;
  }

  // Update modification time + client/server count
  profile.meta.lastModification = std::chrono::system_clock::now();

  switch(entity.meta.type){
    case ET_SV:
      ++profile.meta.svCount;
      break;
    case ET_CL:
      ++profile.meta.clCount;
      break;
    default:
      break;
  }

  // Create inline config file
  std::vector<const Entity*> entityList{&entity};

  bool create_inline = pkiconf["output"]["create_inline"] == "yes" ? true : false;
  bool create_pfx = pkiconf["output"]["create_pfx"] == "yes" ? true : false;

  if(create_inline && __create_inline_config(profile, config, entityList, autoanswer, prompt) == GPKIH_OK){
    PSUCCESS("inline config created\n");
  }

  if(create_pfx &&  __create_pfx(profile,entity) == GPKIH_OK){
    PSUCCESS("pfx file created\n");
  }
  
  return GPKIH_OK;
};

int ANew::exec(std::vector<std::string> &args) const {
  DEBUG(1, "parsers::build()");

  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (args.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
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
  
  if(args.size() == 1){
    PERROR("Entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  std::string_view eTypeStr = args[1];

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

  args.erase(args.begin(),args.begin()+2);
  
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
    }else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }

  if(entity.subject.cn == nullptr){
    // User didn't give common_name (mandatory to build a certificate) with cli args
    auto sub = config.default_subject();  
    subject::promptForSubject(profile->name, entity.subject, sub, eman);
  }

  int rcode = GPKIH_FAIL;
  
  auto pkiconf = config.get(CFILE_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  std::string_view keySize = pkiconf["key"]["size"];
  std::string_view keyAlgo = pkiconf["key"]["algorithm"];
  std::string_view days    = pkiconf["crt"]["days"];

  if(entity.meta.type & ET_CA){
    
    if(entity::setCAPaths(*profile, entity) != GPKIH_OK){
      PERROR("smth failed on setCAPaths()\n");
      return GPKIH_FAIL;
    };

    rcode = build_ca(*profile,config,entity,eman,days,keyAlgo,keySize);
  
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
      
      if(build_ca(*profile, config, newCA, eman, days, keyAlgo, keySize) == GPKIH_FAIL){
        return GPKIH_FAIL;
      }
    }

    if(entity::setPaths(*profile,entity) != GPKIH_OK){
      PERROR("smth failed on setPaths()\n");
      return GPKIH_FAIL;
    }

    rcode = build(*profile,config,entity,eman,days,keyAlgo,keySize);
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