#include "actions.hpp"
using namespace gpki;

static void _get_and_set(std::string &st){
  std::string input;
  std::getline(std::cin,input);
  if(!input.empty()){
    st = std::move(input);
  }
}
static inline str _server_client_csr_command(str &gopenssl, subopts::build &params, Entity &entity){
  return fmt::format("openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc",params.key_size, params.algorithm, entity.req_path,
                     entity.key_path, entity.subject.oneliner(), params.csr_crt_format, params.key_format);
}
static inline str _server_client_crt_command(str &gopenssl, subopts::build &params, Entity &entity){
  return fmt::format("openssl ca -config {} -in {} -out {} -subj '{}' -extfile {}x509{}{} -notext -days +{}",gopenssl,entity.req_path,entity.cert_path,
                     entity.subject.oneliner(), CONF_DIRPATH, SLASH, to_str(entity.type), params.days);
}
static inline int _create_config(Profile &profile, std::vector<Entity> &entities, int _inline = 1){
  str ca_crt_path = profile.ca_crt();   
  if(!fs::exists(ca_crt_path)){
    PERROR("ca cert path '{}' doesn't exist\n", ca_crt_path);
    return -1;
  }
  // Load CA certificate
  int _ca_filesize = fs::file_size(ca_crt_path);
  str ca_crt_str("\0",_ca_filesize);
  std::ifstream(ca_crt_path).read(&ca_crt_str[0],ca_crt_str.size());
  if(ca_crt_str.empty()){
    PERROR("couldn't load CA certificate '{}'\n",ca_crt_path);
    return -1;
  }
  for(auto &entity : entities){
    auto outdir = fmt::format("{}{}packs{}{}",profile.source,SLASH,SLASH,entity.subject.cn);
    if(create_output_path(outdir)){
      return -1;
    };
    str outpath;
    if(_inline){
      outpath = fmt::format("{}{}inline_{}.{}",outdir,SLASH,entity.subject.cn,VPN_CONFIG_EXTENSION);
    }else{
      outpath = fmt::format("{}{}pack_{}",outdir,SLASH,entity.subject.cn);
      if(create_output_path(outpath)){
        return -1;
      };
      // Copy files to pack dir
      fs::copy(entity.key_path,outpath);
      fs::copy(entity.cert_path,outpath);
      fs::copy(ca_crt_path,outpath);
      // Dump config file
      GpkihConfig::dump(fmt::format("{}{}{}.{}",outpath,SLASH,entity.subject.cn,VPN_CONFIG_EXTENSION),entity.type);
      return 0;
    }
    if(GpkihConfig::dump(outpath,entity.type)){
      PWARN("couldn't create config file for entity with CN '{}'\n", entity.subject.cn);
      return -1;
    }
    /* At this point the common & (client|server) config has already been added to the file
     * so just check if */ 
    int _e_crt = fs::file_size(entity.cert_path);
    int _e_key = fs::file_size(entity.key_path);
    str entity_crt_str("\0",_e_crt);
    str entity_key_str("\0", _e_key);
    std::ifstream(entity.cert_path).read(&entity_crt_str[0], entity_crt_str.size());
    std::ifstream(entity.key_path).read(&entity_key_str[0], entity_key_str.size());
    if(entity_crt_str.empty()){
      PERROR("couldn't load entity certificate '{}'\n", entity.cert_path);
      return -1;
    }
    if(entity_key_str.empty()){
      PERROR("couldn't load entity key '{}'\n", entity.key_path);
      return -1;
    }
    std::ofstream file(outpath, std::ios::app);
    if(!file.is_open()){
      PERROR("couldn't open file '{}'\n", outpath);
      return -1;
    }
    // Append the inlined values
    file << "<ca>" << EOL << ca_crt_str << "</ca>" << EOL;
    file << "<cert>" <<  EOL << entity_crt_str <<"</cert>" << EOL;
    file << "<key>" << EOL << entity_key_str << "</key>" << EOL;
    // TODO - check if tls-auth property is set and append it  too
  }
  return 0;
}
static inline int _create_config(str &profile_name, std::vector<str> &common_names, int _inline = 1){
  Profile profile;
  if(db::profiles::load(profile_name,profile)){
    seterror("couldn't load profile '{}'\n", profile_name);
    return -1;
  }
  std::vector<Entity> entities;
  for(auto cn : common_names){
    Entity e;
    if(db::entities::load(profile_name, cn, e)){
      seterror("couldn't load entity with cn '{}' from profile '{}'\n", cn,profile_name);
      return -1;
    };
    entities.emplace_back(e);
  }
  return _create_config(profile,entities,_inline);
}
/* BUILD CA-SERVER-CLIENT CERTIFICATES */
int actions::build(gpki::subopts::build &params){
  Profile &profile = *params.profile;
  Entity entity;
  entity.type = params.type;
  entity.profile_name = profile.name;

  std::string input;
  // Set country name
  PROMPT( "Country Name (2 letter code) [" + entity.subject.country + "]: ");
  std::getline(std::cin, input);
  if (!input.empty() && input.size() == 2) {
    entity.subject.country = input;
  }
  // Set state name
  PROMPT("State or Province Name (full name) [" + entity.subject.state + "]: ");
  _get_and_set(entity.subject.state);
  // Set location
  PROMPT("Locality Name [" + entity.subject.location + "]: ");
  _get_and_set(entity.subject.location);
  // Set organisation
  PROMPT("Organisation Name [" + entity.subject.organisation + "]: ");
  _get_and_set(entity.subject.organisation);
  // *MANDATORY Set common name
  input.assign("");
  PROMPT("Common Name: ");
  std::getline(std::cin, input);
  while (input.empty()) {
    PWARN("common name can't be empty\n");
    PROMPT("please introduce a common name: ");
    std::getline(std::cin, input);
  };
  entity.subject.cn = input;
  // Set email
  PROMPT("Email Address: ");
  _get_and_set(entity.subject.email);
  
  if(db::entities::exists(profile.name,entity.subject.cn)){
    PERROR("Entity with CN '{}' already exists in profile '{}'\n",entity.subject.cn,profile.name);
    return -1;
  }

  std::string gopenssl = profile.source + SLASH + "gopenssl.conf";
  // Load serial number
  str fpath = profile.source + SLASH + "pki" + SLASH + "serial" + SLASH + "serial";
  std::fstream file(fpath,std::ios::in | std::ios::out);  
  if(!file.is_open()){  
    PERROR("couldn't open serial file '{}'\n",fpath);
    return -1;
  }
  file >> entity.serial;
  if(entity.type == ET_CA){
    /* CA */
    entity.req_path = "\0";
    entity.key_path = profile.source + SLASH + "pki" + SLASH + "ca" + SLASH + "key";
    entity.cert_path = profile.source + SLASH + "pki" + SLASH + "ca" + SLASH + "crt";
    str command = fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc -set_serial '{}'",gopenssl,
                              entity.cert_path, entity.key_path, entity.subject.oneliner(), params.csr_crt_format, params.key_format, entity.serial);
    if(system(command.c_str())){
      PERROR("command '{}' FAILED\n", command);
      return -1;
    }

    // Increment serial and update file
    int _s = strtol(entity.serial.c_str(), nullptr, 16);
    ++_s;
    str hex_serial = fmt::format("{:x}",_s);
    file.seekg(SEEK_SET);
    if(hex_serial.size() == 1){
      file << "0";
    }
    file << hex_serial;
    file.flush();
  }else{
    /* CLIENT | SERVER */
    entity.req_path = profile.source + SLASH + "pki" + SLASH + "reqs" +
                      SLASH + entity.subject.cn + "-csr." + params.csr_crt_format;
    entity.key_path = profile.source + SLASH + "pki" + SLASH + "keys" +
                      SLASH + entity.subject.cn + "-key." + params.key_format;
    entity.cert_path = profile.source + SLASH + "pki" + SLASH + "certs" +
                       SLASH + entity.subject.cn + "-crt." + params.csr_crt_format;
    // First command - certificate signing request creation
    auto csr_command = _server_client_csr_command(gopenssl, params, entity);
    if(system(csr_command.c_str())){
        PERROR("command '{}' failed\n", csr_command);
        return -1;
    }
    // Second command - certificate creation signing the previously created certificate signing request
    auto crt_command = _server_client_crt_command(gopenssl, params, entity);
    if(params.autoanswer_yes){
      crt_command += " -batch";
    }
    if(system(crt_command.c_str())){
      PERROR("command '{}' failed\n",crt_command);
      return -1;
    }
  }
  
  //
  if(entity.type & ET_SV || entity.type & ET_CL){
    GpkihConfig::load(profile); // Load profile's gpkih.conf file
    std::vector<Entity> ents{entity};
    if(_create_config(profile, ents)){
      return -1;
    };
  }
  // Entity succesfully added to the pki, add to database
  if(db::entities::add(entity)){
    PERROR("couldn't add entity '{}' to database\n", entity.subject.cn);
    return -1;
  };

  // Increment profile entity count or set ca to active
  switch(entity.type){
    case ET_CA:
      profile.ca_created = 1;
    break;
    case ET_SV:
      ++profile.sv_count;
    break;
    case ET_CL:
      ++profile.cl_count;
    break;
    default:
      PERROR("unexpected error - invalid entity type '{}'\n", to_str(entity.type));
      return -1;
    break;
  }

  fmt::print("profile count: cl:{} sv:{} ca_created:{}\n", profile.cl_count, profile.sv_count, profile.ca_created);
  // Update database
  return db::profiles::sync();
}