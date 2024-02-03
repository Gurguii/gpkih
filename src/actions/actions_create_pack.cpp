#include "actions.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <map>
#include <vector>

#include "../../src/utils/gpkih_util_funcs.hpp"
#include "../../src/structs.hpp"
#include "../../src/db/database.hpp"

using str = std::string;
using sstream = std::stringstream;
using strview = std::string_view;
using Config = std::map<str,str>;

enum class ETYPE {
    sv,
    cl
};

class vpn_config
{
    private:
    static inline str profile;
    static inline Config conf;

    static void load_section(std::ifstream &file, int allow_empty_or_none = 1){
        str line, key, val;
        sstream ss;
        while(getline(file,line)){
            char c = line[0];
            if(c == ' '){
                continue;
            }
            if(c == '#' || c == '['){
                break;
            }
            ss.write(line.c_str(),line.size());
            getline(ss,key,' ');
            getline(ss,val,'\n');
            if(val.empty() && !allow_empty_or_none){
                PWARN("found unset value '{}'\n",val);
                continue;
            }
            conf.emplace(key,val);
            ss.clear();
        }
    }
    public:
    static int set_profile(const char *path){
        profile = path;
        profile.clear();
        conf.clear();
        // Load common config
        std::ifstream file(path);
        str line;
        while(getline(file,line)){
            if(line == "[ common ]"){
                PINFO("loading [ common ] section\n");
                load_section(file);
                break;
            }
        }
        if(conf.empty()){
            // didn't load common section, something happened
            PERROR("couldn't load [ common ] section\n");
            return -1;
        }
        return 0;
    }

    static void get_config(Entity &entity, int split = 0, const char *outdir = nullptr){
        if(profile.empty()){
            PERROR("set_profile() must be called before doing calls to get_config()\n");
            return;
        }
        if(entity.type == "client"){
            switch(split){
                case 0:
                // inline
                break;
                case 1:
                // split
                break;
            }
        }else if(entity.type == "server"){
            switch(split){
                case 0:
                // inline
                break;
                case 1:
                // split
                break;
            }
        }else{
            PERROR("entity type '{}' not suitable for pack creation\n");
            return;
        }
    }
    // Print key - values from the loaded configuration 
    static void print(){
        for(auto kv : conf){
            std::cout << "key: " << kv.first << " value: " << kv.second << "\n";
        }
    }
    
    // Write config file
    static void write(str outpath){
        if(profile.empty()){
            PERROR("path hasn't been set\n");
            PINFO("vpn_config::load(str template) must be called\n");
        }
        std::ofstream file(outpath);
        if(!file.is_open()){
            PERROR("couldn't create file '{}'\n", outpath);
            return;
        }
        for(auto kv : conf){
            file << kv.first << " " << kv.second << std::endl;
        }
    }   

    // Load config file
    static void load(const char *path, ETYPE type){
        if(path == nullptr){
            PERROR("path can't be null\n");
            return;
        }
        if(!conf.empty()){
            conf.clear();
        }
        std::ifstream file(path);
        if(!file.is_open()){
            std::cerr << "couldn't open file " << path << "\n";
            return;
        }
        str line;
        str key, val;
        sstream ss;
        // load [ common ] section
        while(getline(file,line)){
            if(line == "[ common ]"){
                // Load common section
                while(getline(file,line)){
                    load_section(file);
                }
            }
        }
        // load type-specific section (client|server)
       switch(type){
            case ETYPE::cl:
            //  load [ client ] and [ client.optional ] sections
            while(getline(file,line)){
                if(line == "[ client ]" || line == "[ client.optional ]"){
                    while(getline(file,line)){
                        load_section(file);
                    }
                }
            }
            break;
            
            case ETYPE::sv:
            // load [ server ] and [ server.optional ] sections 
            while(getline(file,line)){
                if(line == "[ server ]" || line == "[ server.optional ]"){
                    while(getline(file,line)){
                        load_section(file);
                    }
                }
            }
            break;
        }
    }
    void clear(){
        conf.clear();
    }
    void client_properties(){
        
    }
    void server_properties(){

    }
};

using namespace gpki;
int actions::create_pack(subopts::create_pack &params){
    // ./gpki create-pack [profile] [cn1,cn2...cnX]
    Profile &profile = params.profile;
    str basedir = profile.source + "packs" + SLASH;
    if(create_output_path(basedir)){
        return -1;
    };
    vpn_config::set_profile(profile.source.c_str());
    for(Entity &e : params.entities){
        str outdir = basedir + e.subject.cn;
        if(!fs::create_directory(outdir)){
            PERROR("couldn't create directory '{}'\n",outdir);
            return -1;
        }
        if(e.type == "client"){

        }else if(e.type == "server"){

        }else{
            PERROR("cannot create pack for entity with type '{}'",e.type);
            return -1;
        }
    }
    return 0;
}