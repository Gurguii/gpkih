#include "subparser.hpp"
#include <filesystem>

int subparsers::create_pack(std::vector<str> opts){
    // ./gpki create-pack <profiles> <cn1,cn2...cnX>
    if(opts.empty()){
        PERROR("missing profile name must be given\n");
        PHINT("try gpki help create_pack\n");
        return -1;
    }
    if(opts.size() != 2){
        help::create_pack::usage();
        return -1;
    }
    subopts::create_pack params;
    Profile &profile = params.profile;
    
    if(db::profiles::load(opts[0],profile)){
        PERROR("profile '{}' doesn't exist\n");
        return -1;
    }
    
    sstream ss(opts[1]);
    str cn;
    // Load desired entities
    Entity e;
    while(getline(ss,cn,CSV_DELIMITER_c)){
        if(db::entities::load(profile.name,cn,e)){
            PWARN("entity '{}' not found in profile '{}'\n",cn,profile.name);
            continue;
        }
        params.entities.push_back(e);
    }
    opts.erase(opts.begin(),opts.begin()+2);
    opts.push_back("\0");
    // Parse subopts
    for(int i = 0; i < opts.size(); ++i){
        strview opt = opts[i];
        if(opt == "-o" || opt == "--out"){
            if(!create_output_path(opts[++i])){
                params.outdir = opts[++i];
            }
        }else if(opt == "--inline"){
            params.inline_outfile = 1;           
        }
    }
    return actions::create_pack(params);
}