#include "actions.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <map>
#include <unordered_map>
#include <vector>

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
    vpn_config::set(profile);

    // check that required fields are set
    for(Entity &e : params.entities){
        if(vpn_config::check_required_fields(e.type)){
            return -1;
        }
    }

    return 0;
    for(Entity &e : params.entities){
        str outdir = basedir + e.subject.cn;
        if(!fs::create_directory(outdir)){
            PERROR("couldn't create directory '{}'\n",outdir);
            return -1;
        }
        if(e.type == ET_CL){

        }else if(e.type ==  ET_SV){

        }else{
            PERROR("cannot create pack for entity with type '{}'\n",to_str(e.type));
            return -1;
        }
    }
    return 0;
}