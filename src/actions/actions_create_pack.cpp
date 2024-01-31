#include "actions.hpp"

using namespace gpki;
int actions::create_pack(subopts::create_pack &params){
    str outdir = DBDIR + "packs" + SLASH + params.profile_name;
    if(check_output_path(outdir)){
        return -1;
    };
    for(Entity &e : params.entities){

    }
    return 0;
}