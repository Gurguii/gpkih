#include "actions.hpp"

using namespace gpki;
int actions::create_pack(subopts::create_pack &params){
    // ./gpki create-pack [profile] [cn1,cn2...cnX]
    str outdir = BASEDIR + "packs" + SLASH + params.profile_name;
    if(create_output_path(outdir)){
        return -1;
    };
    for(Entity &e : params.entities){
        if(e.type == "client"){

        }else if(e.type == "server"){

        }else{
            PERROR("cannot create pack for entity with type '{}'",e.type);
            return -1;
        }
    }
    return 0;
}