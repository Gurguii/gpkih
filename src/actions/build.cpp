#include "actions.hpp"
using namespace gpki;

static void _get_and_set(std::string &st){
  std::string input;
  std::getline(std::cin,input);
  if(!input.empty()){
    st = input;
  }
}
static inline str _server_client_csr_command(str &gopenssl, subopts::build &params, Entity &entity){
  return fmt::format("openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc",params.key_size, params.algorithm, entity.req_path,
                     entity.key_path, entity.subject.oneliner(), params.csr_crt_format, params.key_format);
}
static inline str _server_client_crt_command(str &gopenssl, Entity &entity){
  return fmt::format("openssl ca -config {} -in {} -out {} -subj '{}' -extfile {}x509{}{} -notext",gopenssl,entity.req_path,entity.cert_path,
                     entity.subject.oneliner(), CONF_DIRPATH, SLASH, to_str(entity.type));
}
int actions::build(gpki::subopts::build &params){
    Profile &profile = params.profile;
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
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.email = input;
    }
    
    if(db::entities::exists(profile.name,entity.subject.cn)){
      PERROR("Entity with CN '{}' already exists in profile '{}'\n",entity.subject.cn,profile.name);
      return -1;
    }

    std::string gopenssl = profile.source + SLASH + "gopenssl.cnf";
    if(entity.type == ET_CA){
      /* CA */
        entity.req_path = "\0";
        entity.key_path = profile.source + SLASH + "pki" + SLASH + "ca" + SLASH + "key";
        entity.cert_path = profile.source + SLASH + "pki" + SLASH + "ca" + SLASH + "crt";
        str command = fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc",gopenssl,
                                  entity.cert_path, entity.key_path, entity.subject.oneliner(), params.csr_crt_format, params.key_format);
        if(system(command.c_str())){
          PERROR("command '{}' FAILED\n", command);
          return -1;
        }
    }else{
      /* CLIENT | SERVER */
      entity.req_path = profile.source + SLASH + "pki" + SLASH + "reqs" +
                        SLASH + entity.subject.cn + "-csr." + params.csr_crt_format;
      entity.key_path = profile.source + SLASH + "pki" + SLASH + "keys" +
                        SLASH + entity.subject.cn + "-key." + params.key_format;
      entity.cert_path = profile.source + SLASH + "pki" + SLASH + "certs" +
                         SLASH + entity.subject.cn + "-crt." + params.csr_crt_format;
      auto csr_command = _server_client_csr_command(gopenssl, params, entity);
      if(system(csr_command.c_str())){
          PERROR("command '{}' failed\n", csr_command);
          return -1;
      }
      // SECOND COMMAND
      auto crt_command = _server_client_crt_command(gopenssl, entity);
      if(!params.autoanswer_yes){
        crt_command += " -batch";
      }
      if(system(crt_command.c_str())){
          PERROR("command '{}' failed\n",crt_command);
          return -1;
      }
      str fpath = profile.source + SLASH + "pki" + SLASH + "serial" + SLASH + "serial";
      std::ifstream file(fpath);  
      if(!file.is_open()){  
        PERROR("couldn't open file '{}'\n",fpath);
        return -1;
      }
      file >> entity.serial; 
      file.close();
  }
  // Entity succesfully added to the pki, add to database
  return db::entities::add(entity);
}
