#include "actions.hpp"

#include <filesystem>
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
    VpnConfig::set(profile);
    // check that required fields are set
    for(Entity &e : params.entities){
        if(VpnConfig::check_required_fields(e.type)){
            return -1;
        }
    }
    for(Entity &e : params.entities){
        str outdir = basedir + e.subject.cn;
        str outfile = outdir + SLASH + e.subject.cn + ".ovpn";
        if(!fs::is_directory(outdir)){
            if(!fs::create_directory(outdir)){
                PERROR("couldn't create directory '{}'\n",outdir);
                return -1;
            }
        }
        if(VpnConfig::dump(outfile, e.type)){
            PERROR("couldn't create '{}' pack\n",e.subject.cn);
            continue;
        };
    }
    return 0;
}