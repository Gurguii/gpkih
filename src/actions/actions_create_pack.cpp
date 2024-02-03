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

using str = std::string;
using sstream = std::stringstream;
using strview = std::string_view;
using Section = std::unordered_map<str,str>;
#define map std::map

enum class ETYPE {
    sv,
    cl
};

class VpnConfig{
    public:
    static inline Section common;
    
    static inline Section client;
    static inline Section client_optional;

    static inline Section server;
    static inline Section server_optional;
    


    static void clear(){
        common.clear();
        client.clear();
        client_optional.clear();
        server.clear();
        server_optional.clear();
    }
    static inline int empty(){
        return common.empty() | client.empty() | client_optional.empty() | server.empty() | server_optional.empty();
    }
    static inline bool is_valid_section(str &line){
        return section_mapping.find(line) != section_mapping.end();
    }
    static inline Section* get_section_ref(str &section){
        return section_mapping[section];
    }
    private:
    static inline std::unordered_map<str, Section*>section_mapping{
        {"[ common ]",&common},
        {"[ client ]",&client},
        {"[ client.optional ]",&client_optional},
        {"[ server ]",&server},
        {"[ server.optional ]",&server_optional}
    };
};

class vpn_config
{
    private:
    static inline str profile;
    static inline VpnConfig conf{};
    // Returns position to line where next section is
    static int load_section(std::ifstream &file, Section &section, int allow_empty_or_none = 1){
        str line, key, val;
        sstream ss;
        std::streampos pos;
        while(getline(file,line)){
            char c = line[0];
            if(line.empty() || c == '#'){
                // empty / commented line, not interested
                continue;
            }
            if(c == '['){
                // reached new section
                return pos;
            }
            ss.write(line.c_str(),line.size());
            getline(ss,key,' ');
            getline(ss,val,'\n');
            if(val.empty()){
                PWARN("found unset value '{}'\n",key);
                continue;
            }
            section.emplace(key,val);
            ss.clear();
            pos = file.tellg();
        }
        return 0;
    }
    public:
    static int set(Profile &profile){
        if(!conf.empty()){
            conf.clear();
        }
        str path = profile.source + SLASH + template_filename;
        
        // Load config
        std::ifstream file(path);
        if(!file.is_open()){
            PERROR("couldn't open config file '{}'\n",path);
            return -1;
        }
        str line;

        while(getline(file,line)){
            if(VpnConfig::is_valid_section(line)){
                // Got section
                PINFO("Loading section '{}'\n",line);
                Section *ptr = VpnConfig::get_section_ref(line);
                int next_section = load_section(file,*ptr);
                for(auto kv : *ptr){
                    std::cout << "key: " << kv.first << "  value: " << kv.second << "\n";
                }
                file.seekg(next_section);
            }
        }
        return 0;
    }
};

using namespace gpki;
int actions::create_pack(subopts::create_pack &params){
    // ./gpki create-pack [profile] [cn1,cn2...cnX]
    Profile &profile = params.profile;
    str basedir = profile.source + SLASH + "packs" + SLASH;

    // set profile 
    vpn_config::set(profile);

    return 0;
    for(Entity &e : params.entities){
        str outdir = basedir + e.subject.cn;
        if(!fs::create_directory(outdir)){
            PERROR("couldn't create directory '{}'\n",outdir);
            return -1;
        }
        if(e.type == "client"){

        }else if(e.type == "server"){

        }else{
            PERROR("cannot create pack for entity with type '{}'\n",e.type);
            return -1;
        }
    }
    return 0;
}