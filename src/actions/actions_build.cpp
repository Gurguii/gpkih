#include "actions.hpp"
#include <cstddef>
using namespace gpki;
using subopts = gpki::subopts::build;


int actions::build(gpki::subopts::build &opts){
    Profile &profile = opts.profile;
    Entity entity;
    entity.type = opts.type;
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
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.state = input;
    }
    // Set location
    PROMPT("Locality Name [" + entity.subject.location + "]: ");
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.location = input;
    }
    // Set organisation
    PROMPT("Organisation Name [" + entity.subject.organisation + "]: ");
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.organisation = input;
    }
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
        std::string command = "openssl req"
        " -config " + gopenssl + 
        " -new -x509"
        " -out " + entity.cert_path + 
        " -keyout " + entity.key_path + 
        " -subj '" + entity.subject.oneliner() + "'" 
        " -outform " + opts.csr_crt_format + 
        " -keyform " + opts.key_format +
        " -noenc";
        if(system(command.c_str())){
            std::cout << "command '" << command << "'\n";
            return -1;
        }
    }else{
      /* CLIENT / SERVER */
      entity.req_path = profile.source + SLASH + "pki" + SLASH + "reqs" +
                        SLASH + entity.subject.cn + "-csr." + opts.csr_crt_format;
      entity.key_path = profile.source + SLASH + "pki" + SLASH + "keys" +
                        SLASH + entity.subject.cn + "-key." + opts.key_format;
      entity.cert_path = profile.source + SLASH + "pki" + SLASH + "certs" +
                         SLASH + entity.subject.cn + "-crt." + opts.csr_crt_format;
      std::string csr_command = "openssl req" 
      " -newkey " + opts.key_size + ":" + opts.algorithm + 
      " -out " + entity.req_path + 
      " -keyout " + entity.key_path + 
      " -subj '" + entity.subject.oneliner() + "'" + 
      " -outform " + opts.csr_crt_format + 
      " -keyform " + opts.key_format +
      " -noenc";
      if(system(csr_command.c_str())){
          PERROR("command '{}' failed\n", csr_command);
          return -1;
      }
      // SECOND COMMAND
      std::string crt_command = "openssl ca" 
      " -config " + gopenssl + 
      " -in " + entity.req_path + 
      " -out " + entity.cert_path +
      " -subj '" + entity.subject.oneliner() + "'"
      " -extfile " + CONFDIR + SLASH + "x509" + SLASH + to_str(entity.type) +
      " -notext";
      if(!prompt){
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