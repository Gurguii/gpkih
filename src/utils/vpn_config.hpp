#pragma once
#include <fstream>
#include <cstdio>

#include "../gpki.hpp"
#include "../printing.hpp"
#include "../structs.hpp"

using str = std::string;
using sstream = std::stringstream;
using strview = std::string_view;
using Section = std::unordered_map<str,str>;
#define map std::map

static inline str SECTION_COMMON = "[ common ]";
static inline str SECTION_CLIENT = "[ client ]";
static inline str SECTION_CLIENT_OPT  = "[ client.optional ]";
static inline str SECTION_SERVER = "[ server ]";
static inline str SECTION_SERVER_OPT  = "[ server.optional ]";

namespace gpki
{

class VpnConfig{
    public:
    static inline Section common{};
    
    static inline Section client{};
    static inline Section client_optional{};

    static inline Section server{};
    static inline Section server_optional{};

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
};

class vpn_config
{
    private:
    static inline std::unordered_map<str, Section>section_mapping{
        {SECTION_COMMON,VpnConfig::common},
        {SECTION_CLIENT,VpnConfig::client},
        {SECTION_CLIENT_OPT,VpnConfig::client_optional},
        {SECTION_SERVER,VpnConfig::server},
        {SECTION_SERVER_OPT,VpnConfig::server_optional}
    };
    static inline bool is_valid_section(str &line){
        return section_mapping.find(line) != section_mapping.end();
    }
    static inline int _set = 0;
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
    /* Loads config from given profile's template.conf inside VpnConfig Sections
    which is an alias for unordered_map<string,string> holding {common | client | server} configuration */
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

        // Load templates.conf sections
        while(getline(file,line)){
            if(is_valid_section(line)){
                // Got section
                // PINFO("Loading section '{}'\n",line);
                Section &ref = section_mapping[line];
                int next_section = load_section(file,ref);
                file.seekg(next_section);
            }
        }
        _set = 1;
        return 0;
    }

    static int check_required_fields(ENTITY_TYPE type){
        if(!_set){
            PERROR("vpn_config::set() must be called first in order to load adecuate templates.conf\n");
            return -1;
        }
        int rcode = 0;
        if(type == ET_CL){
            std::cout << "CHECKING CLIENT FIELDS\n";
            Section &ref = section_mapping[SECTION_CLIENT];
            for(auto &kv : ref){
                if(kv.second == "UNSET"){
                    rcode = -1;
                    PWARN("mandatory field '{}' not set\n", kv.first);
                }
            }
        }else if(type == ET_SV){
            std::cout << "CHECKING SERVER FIELDS\n";
            Section &ref = section_mapping[SECTION_SERVER];
            for(auto &kv : ref){
                if(kv.second == "UNSET"){
                    rcode = -1;
                    PWARN("mandatory field '{}' not set\n", kv.first);
                }
            }
        }else{
            PWARN("entity must be a client | server in order to create a pack\n");
            return -1;
        }
        return rcode;
    }
};
} // namespace gpki