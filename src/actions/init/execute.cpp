#include "AInit.hpp"

#include <fmt/format.h>

#include "../actions.hpp"

#include "../../db/profiles.hpp"
#include "../../entities/subj_utils.hpp"

#include "../../libs/utils/utils.hpp"
#include "../../libs/printing/printing.hpp"

#include "../../config/Config.hpp"
#include "../../config/ProfileConfig.hpp"

#include "../../gpkih.hpp"

#include "../../libs/ssl/pkey/dh.hpp"

constexpr const char *opensslConfFilename = "gopenssl.conf";

static inline std::vector<std::string> RELATIVE_DIRECTORY_PATHS(){
  return 
  {
    "packs",
    "logs",
    fmt::format("pki{}ca", SLASH),
    fmt::format("pki{}certs", SLASH),
    fmt::format("pki{}keys", SLASH),
    fmt::format("pki{}reqs", SLASH),
    fmt::format("pki{}serial", SLASH),
    fmt::format("pki{}database", SLASH),
    fmt::format("pki{}crl", SLASH),
    fmt::format("pki{}tls",SLASH)                                                                                       
  };
}

static inline std::unordered_map<std::string,std::string> RELATIVE_FILE_PATHS(){
  return 
  {  
    {fmt::format("pki{}crl{}crlnumber", SLASH, SLASH), "1000"},
    {fmt::format("pki{}database{}index.txt", SLASH, SLASH), ""},
    {fmt::format("pki{}serial{}serial", SLASH, SLASH), "01"}                                                         
  };
}  

static bool __has_write_perms(std::string dirpath) {
  DEBUG(2, "__has_write_perms()");

  /* this approach is kind of sad to see */
  try {
    auto s = fs::status(dirpath);
    return true;
  } catch (fs::filesystem_error &ex) {
    PERROR(ex.what());
    return false;
  }
}

/* __sed("/home/gurgui/base", "/home/gurgui/after_sed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})
*/
static int __sed(std::string_view src, std::string_view dst,
  std::unordered_map<std::string_view, std::string_view> &&vals) {
  DEBUG(2, "__sed()");

  std::ifstream srcfile(src.data());
  if (!srcfile.is_open()) {
    return GPKIH_FAIL;
  }
  std::ofstream dstfile(dst.data());
  if (!dstfile.is_open()) {
    return GPKIH_FAIL;
  }
  std::string line;
  std::string word;
  while (getline(srcfile, line)) {
    auto ss = std::stringstream(line);
    while (ss >> word) {
      if (vals.find(word) != vals.end()) {
        dstfile << vals[word];
      } else {
        dstfile << word;
      }
      dstfile << " ";
    }
    dstfile << EOL;
  }
  dstfile << EOL;
  srcfile.close();
  dstfile.close();
  return GPKIH_OK;
}                                                  

template <typename T> static int __is_valid_path(T path) {
  DEBUGF(2, "__is_valid_path({})", path);

  if (gurgui::utils::fs::is_absolute_path(path) == false) {
    return GPKIH_FAIL;
  };
  
  try {
    if (fs::exists(path)) {
      auto ans = PROMPT("File or directory already exists, remove?", "[y/n]", true);
      if (ans == "y" || ans == "yes") {
        fs::remove_all(path);
        if(fs::exists(path) == true){
          PERROR("Couldn't remove '{}' recursively",path);
          return GPKIH_FAIL;  
        }
      }
      return GPKIH_OK;
    };
  } catch (std::exception ex) {
    PERROR("Permission denied in '{}' - {}\n", path, ex.what());
    return GPKIH_OK;
  }

  return GPKIH_OK;
}

using namespace gpkih;
int __create_pki_filestruct(Profile &profile){
  DEBUG(2, "__create_pki_filestruct()");

  // Create directories
  for (const std::string &relative : RELATIVE_DIRECTORY_PATHS()) {
    std::string path = fmt::format("{}{}{}",profile.source, SLASH, relative);
  
    if (!fs::create_directories(path)) {
      PERROR("Couldn't create directory '{}'", path);
      // Remove the profile source dir
      // fs::remove_all(profile.source);
      return GPKIH_FAIL;
    }
  }
  // Create files
  for (const std::pair<std::string, std::string> &p : RELATIVE_FILE_PATHS()) {
    // p.first -> path
    // p.second -> default file contents
    std::string path = fmt::format("{}{}{}",profile.source,SLASH,p.first);

    std::ofstream(path, std::ios::app).write(p.second.c_str(), p.second.size());
    if (!fs::exists(path)) {
      PERROR("Couldn't create file {}",p.first);
      return GPKIH_FAIL;
    }
  }

  // Copy required config files to profile
  for (const auto &filename :
       {vpnConfFilename, pkiConfFilename}) {
    std::string src = fmt::format("{}{}", GPKIH_DIR_CONFIG, filename);
    std::string dst = fmt::format("{}{}{}", profile.source, SLASH, filename);
    fs::copy(src, dst);
    if (!fs::exists(dst)) {
      PERROR("Couldn't copy '{}' to '{}'\n", src, dst);
      return GPKIH_FAIL;
    }
  }
  
  std::string gopenssl_sed_src = fmt::format("{}{}",GPKIH_DIR_CONFIG,opensslConfFilename);
  std::string gopenssl_sed_dst = fmt::format("{}{}{}",profile.source,SLASH,opensslConfFilename);

// [Windows] - change \ for / since openssl processes slashes as / in the openssl.conf file
#ifdef _WIN32
  utils::str::replace_if(profile.source,utils::str::length(profile.source),'\\','/');
#endif

// [Linux | Windows] - adapt gopenssl.cnf 
if (__sed(gopenssl_sed_src, gopenssl_sed_dst, {{"GPKIH_PATH_ROOTDIR", fmt::format("{}/pki",profile.source)}})) {
  PERROR("__sed failed()\n");
  return GPKIH_FAIL; 
}

// [Windows] - change '/' slashes back to '\'
#ifdef _WIN32
  utils::str::replace_if(profile.source,utils::str::length(profile.source),'/','\\');
#endif

  // Add profile to database
  if (db::profiles::add(profile)) {
    // error is set by db::profiles::add
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
};

static int __check_profile_info(std::string_view &profileName, std::string_view &profileSource, Profile &profile){
  /* 
    profileName can't be empty
    profile with name `profileName` can't already exist
    profileName size is max 254 characters
  */
  if (profileName.empty() || db::profiles::exists(profileName) || profileName.size() > GPKIH_MAX_VARCHAR) {
    std::string pname;
    for(;;){
      pname = PROMPT("Desired profile name");
      if (db::profiles::exists(pname)) {
        PWARN("profile '{}' already exists\n", pname);
        continue;
      } else if (pname.empty()) {
        PWARN("profile name can't be empty\n");
        continue;
      }else if(pname.size() > GPKIH_MAX_VARCHAR){
        PWARN("max allowed length: {} characters\n", GPKIH_MAX_VARCHAR);
        continue;
      }
      CALLOCATE(profile.name,reinterpret_cast<size_t*>(&profile.meta.nameLen),pname);
      break;
    }
  } else {
    CALLOCATE(profile.name,reinterpret_cast<size_t*>(&profile.meta.nameLen),profileName);
  }

  if(profileSource.empty() || __is_valid_path(profileSource) || profileSource.size() > GPKIH_MAX_PATH)
  {
    std::string_view pSource{profileSource};
    for(;;){
      pSource = PROMPT("Profile source dir (absolute path)", true);
      if(pSource.empty()){
        PWARN("Empty profile source\n");
      }else if(__is_valid_path(pSource)){
        PWARN("Couldn't set profile source\n");
      }else if(pSource.size() > GPKIH_MAX_PATH){
        PWARN("Profile source length can't exceed {}\n", GPKIH_MAX_PATH);
      }else{
        CALLOCATE(profile.source, reinterpret_cast<size_t*>(&profile.meta.sourceLen), pSource);
        break;
      }
      continue;
    }  
  }else{
    CALLOCATE(profile.source, reinterpret_cast<size_t*>(&profile.meta.sourceLen), profileSource);
  }
  
  // Check that we have write permissions in such path
  if (!__has_write_perms(profile.source)) {
    PERROR("no write permissions in '{}'", profile.source);
    return GPKIH_FAIL;
  };

  // Create source dir and any intermediary one in the path
  return fs::create_directories(profile.source) == true ? GPKIH_OK : GPKIH_FAIL;
} // check_profile_info()

static int initializeProfile(std::string_view &profileName, std::string_view &profileSource) {
  Profile profile{};
  profile.meta.lastModification = profile.meta.creationDate = std::chrono::system_clock::now();
  
  if(__check_profile_info(profileName, profileSource, profile) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  if(__create_pki_filestruct(profile) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  // Create profile entities file
  EntityManager eman{profileName};
  
  // sync() profiles.data with db::profiles::existing_profiles to add the new one
  db::profiles::sync();
  
  // Extra questions
  if (Config::get("behaviour", "prompt") == "yes") {
    bool autoans = Config::get("behaviour", "autoanswer") == "no" ? false : true;
    ProfileConfig pconf(profile, CFILE_PKI);
    
    auto pkiconf = pconf.getptr(CFILE_PKI);
    std::string_view keySize;
    std::string_view keyAlgo;
    std::string_view days;

    Entity ca{};
    ca.meta.type = ET_CA;
    bool caBuilt = false;

    try{
      keySize = pkiconf->at("key").at("size");
      keyAlgo = pkiconf->at("key").at("algorithm");
      days    = pkiconf->at("crt").at("days");
    }catch(const std::out_of_range &err){
      PERROR("Retrieving pki.key.size pki.key.algo pki.crt.days - {}\n",err.what());
      return GPKIH_FAIL;
    }

    if (autoans) {
      // QUESTION 1 - create dhparam?
      std::string path = fmt::format("{}{}pki{}tls{}dhparam", profile.source, SLASH, SLASH, SLASH); 

      PINFO("Generating DH parameters...\n");
      gssl::dhparam::DHparam *params = gssl::dhparam::generate();
      
      if( params != nullptr){
        PSUCCESS("Diffie-Hellman param created - {}\n", path);
        delete(params);
      }
      
      // QUESTION 2 - create ca?
      // also had to manually set the type
      auto sub = pconf.default_subject();
      subject::promptForSubject(profile.name, ca.subject, sub, eman);
      entity::setCAPaths(profile,ca);
      entity::loadSerial(profile,ca);
      entity::setExpirationDate(ca,std::strtoull(days.data(), nullptr, 10));

      //if(build_ca(profile, pconf, ca, eman, days, keyAlgo, keySize) == GPKIH_OK){
      //  caBuilt = true;
      //};
      
    }else{
      auto ans = PROMPT("Create dhparam? " + fmt::format(fg(fmt::terminal_color::bright_green) |
                                                  EMPHASIS::underline,
                                              "(highly recommended)"),
             "[y/n]", true);
      if (ans == "y" || ans == "yes") {
        std::string path = fmt::format("{}{}pki{}tls{}dhparam", profile.source, SLASH, SLASH, SLASH);
        PINFO("Generating DH parameters...\n");
        fflush(stdout);
        gssl::dhparam::DHparam *params = gssl::dhparam::generate();
        if(params == nullptr){
          PERROR("Something failed creating DH params\n");
          return GPKIH_FAIL;
        }
        FILE* file = fopen(path.data(), "wb");

        if(file == nullptr){
          PERROR("Couldn't open output file\n");
          return GPKIH_FAIL;  
        }

        if(params->dump(file) == GPKIH_OK){
          PSUCCESS("Diffie-Hellman param created - {}\n", path);
        };

        fclose(file);
        delete(params);
        //if(createDH(path, 4096) == GPKIH_OK){
        //  PSUCCESS("Diffie-Hellman param created - {}\n", path);
        //};
      }

      ans = PROMPT("Create CA?","[y/n]",true);

      // TODO - fix this shit
      if(ans == "y" || ans == "yes"){
        auto sub = pconf.default_subject();
        subject::promptForSubject(profile.name, ca.subject, sub, eman);
        entity::setCAPaths(profile,ca);
        entity::loadSerial(profile,ca);
        entity::setExpirationDate(ca, std::strtoull(days.data(), nullptr, 10));
        //if(build_ca(profile, pconf, ca, eman, days, keyAlgo, keySize) == GPKIH_OK){
        //  caBuilt = true;
        //};
      }
    }
    
    if(caBuilt == true){
      auto &map = *db::profiles::get();
      // Check again to ensure
      if(map.find(profile.name) != map.end()){
        map[profile.name].meta.caCreated = true;  
        return GPKIH_FAIL;
      }
      eman.add(ca);
      eman.sync();
      db::profiles::sync();
    }
  }

  ADD_LOG(LL_INFO,fmt::format("profile:{} action:init source:{}",profile.name,profile.source));
  PSUCCESS("Profile '{}' created\n", profile.name);
  return GPKIH_OK;
}

/* Entrypoint */
int AInit::exec(std::vector<std::string> &args) const {

	/* BEG - Parse arguments */
	DEBUG(1, "AInit::exec()");
  
  	std::string_view profileName, profileSource;
  	for (int i = 0; i < args.size() - 1; ++i) {
  	  std::string_view opt = args[i];
  	  if (opt == "-n" || opt == "--name") {
  	    profileName = args[++i];
  	  } else if (opt == "-s" || opt == "--source") {
  	    profileSource = args[++i];
  	  } else {
  	    UNKNOWN_OPTION_MESSAGE(opt);
  	  }
  	}
	/* END - Parse arguments */
	
  initializeProfile(profileName,profileSource);
	
  return GPKIH_OK;
}