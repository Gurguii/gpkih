#include "build.hpp"

using namespace gpki;

int modes::build::ca(Profile *profile, Entity *entity, build_params *params) { 
  auto commands = get_openssl_command(profile,entity,params);
  if(commands.has_value()){
    for(auto command : commands.value()){
      if(system(command.c_str())){
        std::cout << "[FAILED] " << command << "\n"; 
      };
    }
    // add entity to csv
    return db::entities::add(entity);
  }
  return -1;
}
int modes::build::server(Profile *profile,Entity *entity, build_params *params) { return 0; }
int modes::build::client(Profile *profile,Entity *entity, build_params *params) { return 0; }
int modes::build::get_entity(Profile *profile, Entity *entity, build_params *params){
    std::string input;
    // Set country name
    std::cout << "Country Name (2 letter code) [" << entity->subject.country << "]: ";
    std::getline(std::cin, input);
    
    if (!input.empty()) {
      entity->subject.country = input;
    }
    // Set state name
    std::cout << "State or Province Name (full name) [" << entity->subject.state << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity->subject.state = input;
    }
    // Set location
    std::cout << "Locality Name [" << entity->subject.location << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity->subject.location = input;
    }
    // Set organisation
    std::cout << "Organisation Name [" << entity->subject.organisation << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity->subject.organisation = input;
    }
    // *MANDATORY Set common name
    input.assign("");
    std::cout << "Common Name: ";
    std::getline(std::cin, input);
    while (input.empty()) {
      std::cout << "please introduce a common name: ";
      std::getline(std::cin, input);
    };
    entity->subject.cn = input;

    // Set email
    std::cout << "Email Address: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity->subject.email = input;
    }
    
    // set files paths
    entity->req_path = profile->source + SLASH + "pki" + SLASH + "reqs" +
                      SLASH + entity->subject.cn + "-csr." + params->out_format;
    entity->key_path = profile->source + SLASH + "pki" + SLASH + "keys" +
                      SLASH + entity->subject.cn + "-key." + params->out_format;
    entity->cert_path = profile->source + SLASH + "pki" + SLASH + "certs" +
                       SLASH + entity->subject.cn + "-crt." + params->out_format;

    return 0;
}
std::optional<std::vector<std::string>> modes::build::get_openssl_command(Profile *profile, Entity *entity, build_params *params){
  std::string gopenssl = profile->source + SLASH + "gopenssl.cnf";
  if(entity->type == "ca"){
    return std::vector<std::string>{"openssl req -config " + gopenssl + " -new -x509 -out " + entity->cert_path + " -keyout " + entity->key_path + " -subj '" + entity->subject.oneliner() + "' -noenc"};
  }else{
    // Both client|server certiticates are created with the same command, the only thing that changes are the x509 extensions
    return std::vector<std::string>{"openssl req -newkey " + params->key_size + ":" + params->algorithm + " -out " + entity->req_path + " -keyout " + entity->key_path + " -subj " + entity->subject.oneliner() + " -noenc",
    "openssl ca -config " + gopenssl + " -in " + entity->req_path + " -out " + entity->cert_path + " -subj '" + entity->subject.oneliner() + "' extfile " + globals::configdir + SLASH + entity->type};
  }
}