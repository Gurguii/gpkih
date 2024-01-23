#include "actions.hpp"
using namespace gpki;
using subopts = gpki::subopts::build;
 

int actions::build(Profile *profile, gpki::subopts::build *opts, ENTITY_TYPE type){
    Entity entity;
    entity.type = entity_type_str(type);
    entity.profile_name = profile->name;

    std::string input;
    // Set country name
    std::cout << "Country Name (2 letter code) [" << entity.subject.country << "]: ";
    std::getline(std::cin, input);
    
    if (!input.empty() && input.size() == 2) {
      entity.subject.country = input;
    }
    // Set state name
    std::cout << "State or Province Name (full name) [" << entity.subject.state << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.state = input;
    }
    // Set location
    std::cout << "Locality Name [" << entity.subject.location << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.location = input;
    }
    // Set organisation
    std::cout << "Organisation Name [" << entity.subject.organisation << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.organisation = input;
    }
    // *MANDATORY Set common name
    input.assign("");
    std::cout << "Common Name: ";
    std::getline(std::cin, input);
    while (input.empty()) {
      std::cout << "please introduce a common name: ";
      std::getline(std::cin, input);
    };
    entity.subject.cn = input;

    // Set email
    std::cout << "Email Address: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
      entity.subject.email = input;
    }
   
    if(entity.type == "ca"){
        entity.req_path = "\0";
        entity.key_path = profile->source + SLASH + "pki" + SLASH + "ca" + SLASH + "key";
        entity.cert_path = profile->source + SLASH + "pki" + SLASH + "ca" + SLASH + "crt";
    }else{
        entity.req_path = profile->source + SLASH + "pki" + SLASH + "reqs" +
                          SLASH + entity.subject.cn + "-csr." + opts->csr_crt_format;
        entity.key_path = profile->source + SLASH + "pki" + SLASH + "keys" +
                          SLASH + entity.subject.cn + "-key." + opts->key_format;
        entity.cert_path = profile->source + SLASH + "pki" + SLASH + "certs" +
                           SLASH + entity.subject.cn + "-crt." + opts->csr_crt_format;
    }

    std::string gopenssl = profile->source + SLASH + "gopenssl.cnf";
    if(entity.type == "ca"){
        std::string command = "openssl req"
        " -config " + gopenssl + 
        " -new -x509"
        " -out " + entity.cert_path + 
        " -keyout " + entity.key_path + 
        " -subj '" + entity.subject.oneliner() + "'" 
        " -outform " + opts->csr_crt_format + 
        " -keyform " + opts->key_format +
        " -noenc";
        if(system(command.c_str())){
            std::cout << "command '" << command << "'\n";
            return -1;
        }
    }else{
        // Both client|server certiticates are created with the same command, the only thing that changes are the x509 extensions
        // FIRST COMMAND
        std::string csr_command = "openssl req" 
        " -newkey " + opts->key_size + ":" + opts->algorithm + 
        " -out " + entity.req_path + 
        " -keyout " + entity.key_path + 
        " -subj '" + entity.subject.oneliner() + "'" + 
        " -outform " + opts->csr_crt_format + 
        " -keyform " + opts->key_format +
        " -noenc";
        if(system(csr_command.c_str())){
            std::cout << "[FAIL] - command '" << csr_command << "'\n";
            return -1;
        }
        // SECOND COMMAND
        std::string crt_command = "openssl ca" 
        " -config " + gopenssl + 
        " -in " + entity.req_path + 
        " -out " + entity.cert_path +
        " -subj '" + entity.subject.oneliner() + "'"
        " -extfile " + CONFDIR + SLASH + "x509" + SLASH + entity.type +
        " -notext";
        if(system(crt_command.c_str())){
            std::cout << "[FAIL] - command '" << crt_command << "'\n";
            return -1;
        }
    }   
    // Entity succesfully added to the pki, add to database
    return db::entities::add(&entity);
}
