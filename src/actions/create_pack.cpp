#include "actions.hpp"

#include <filesystem>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>

#include "../../src/utils/gpkih_util_funcs.hpp"
#include "../../src/structs.hpp"
#include "../../src/db/database.hpp"
#include "../utils/vpn_config.hpp"

using namespace gpki;
int actions::create_pack(subopts::create_pack &params){
    // ./gpki create-pack [profile] [cn1,cn2...cnX]
    Profile &profile = params.profile;
    str basedir = profile.source + SLASH + "packs" + SLASH;
    // set profile 
    VpnConfig::set(profile);
    // check that required fields are set
    for(Entity &e : params.entities){
        if(VpnConfig::check_required_fields(e.type)){
            return -1;
        }
    }
    for(Entity &e : params.entities){
        str outdir = basedir + e.subject.cn + SLASH;
        str outfile = outdir + e.subject.cn + ".ovpn";
        if(create_output_path(outdir)){
            PERROR("couldn't create directory '{}'\n");
            continue;
        }
        // Write the entity type configuration into the vpn config file
        if(VpnConfig::dump(outfile, params.profile, e)){
            PERROR("couldn't create '{}' pack\n",e.subject.cn);
            continue;
        };
        str ca_crt_path = profile.source + SLASH + "pki" + SLASH + "ca" + SLASH + "crt";
        if(params.inline_outfile){
            // Load the contents of the ca certificate and entity certificate|key
            int _ca = fs::file_size(ca_crt_path);
            int _e_crt = fs::file_size(e.cert_path);
            int _e_key = fs::file_size(e.key_path);
            str ca_crt_str("\0",_ca);
            str entity_crt_str("\0",_e_crt);
            str entity_key_str("\0", _e_key);
            std::ifstream(ca_crt_path).read(&ca_crt_str[0],ca_crt_str.size());
            std::ifstream(e.cert_path).read(&entity_crt_str[0], entity_crt_str.size());
            std::ifstream(e.key_path).read(&entity_key_str[0], entity_key_str.size());
            if(ca_crt_str.empty()){
                PERROR("couldn't load CA certificate '{}'\n",ca_crt_path);
                return -1;
            }
            if(entity_crt_str.empty()){
                PERROR("couldn't load entity certificate '{}'\n", e.cert_path);
                return -1;
            }
            if(entity_key_str.empty()){
                PERROR("couldn't load entity key '{}'\n", e.key_path);
                return -1;
            }
            std::ofstream file(outfile, std::ios::app);
            if(!file.is_open()){
                PERROR("couldn't open file '{}'\n", outfile);
                return -1;
            }
            // Append the inlined values
            file << "<ca>" << EOL << ca_crt_str << "</ca>" << EOL;
            file << "<cert>" <<  EOL << entity_crt_str <<"</cert>" << EOL;
            file << "<key>" << EOL << entity_key_str << "</key>" << EOL;
            // TODO - check if tls-auth property is set and append it  too
            return 0;
        }
        // Append a file for ca.crt and entity crt|key 
        try{
            fs::copy(ca_crt_path,outdir + "ca.crt");
            fs::copy(e.cert_path, outdir + e.subject.cn + ".crt");
            fs::copy(e.key_path, outdir + e.subject.cn + ".key");
        }catch(fs::filesystem_error err){
            PERROR("{}\n", err.what());
            return -1;
        }
        // TODO - check if tls-auth property is set and append it  too
    }
    return 0;
}
