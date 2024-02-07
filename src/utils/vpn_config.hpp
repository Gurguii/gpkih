#pragma once
#include <fstream>
#include <cstdio>
#include <unordered_map>
#include <filesystem>
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
    static inline int set_common_prop(str &key, str &nval)
    {
        if(common.find(key) != common.end()){
            common[key] = nval; 
        }else{
            return -1;
        }
        return 0;
    }
    static inline int set_client_prop(str &key, str &nval){
        if(client.find(key) != client.end()){
            client[key] = nval;
        }else if(client_optional.find(key) != client_optional.end()){
            client_optional[key] = nval;
        }else{
            return -1;
        }
        return 0;
    }
    static inline int set_server_prop(str &key, str &nval){
        if(server.find(key) != server.end()){
            server[key] = nval;
        }else if(server_optional.find(key) != server_optional.end()){
            server_optional[key] = nval;
        }else{
            return -1;
        }
        return 0;
    }
    static inline int empty(){
        return common.empty() | client.empty() | client_optional.empty() | server.empty() | server_optional.empty();
    }
    static inline bool client_prop_exists(str &key){
        return client.find(key) != client.end() || client_optional.find(key) != client_optional.end();
    }
    static inline bool server_prop_exists(str &key){
        return server.find(key) != server.end() || server_optional.find(key) != server_optional.end();
    }
    static inline bool common_prop_exists(str &key){
        return common.find(key) != common.end();
    }
    static inline Section* prop_exists(str &key){
        if(common.find(key) != common.end()){
            return &common;
        }else if(client.find(key) != client.end()){
            return &client;
        }else if(client_optional.find(key) != client_optional.end()){
            return &client_optional;
        }else if(server.find(key) != server.end()){
            return &server;
        }else if(server_optional.find(key) != server_optional.end()){
            return &server_optional;
        }else{
            return nullptr;
        }
    }
    static inline int set_prop(str &key,str &nval){
        if(common.find(key) != common.end()){
            common[key] = nval;
        }else if(client.find(key) != client.end()){
            client[key] = nval;
        }else if(client_optional.find(key) != client_optional.end()){
            client_optional[key] = nval;
        }else if(server.find(key) != server.end()){
            server[key] = nval;
        }else if(server_optional.find(key) != server_optional.end()){
            server_optional[key] = nval;
        }else{
            return -1;
        }
        return 0;
    }

    static int dump(strview outfile, ENTITY_TYPE type){
        std::ofstream file(outfile.data());
        if(!file.is_open()){
            PERROR("couldn't create '{}'\n",outfile);
            return -1;
        }   
        std::vector<Section *> cfg;
        switch(type){
            case ET_CL:
            cfg = {&common, &client, &client_optional};
            break;
            case ET_SV:
            cfg = {&common,&server,&server_optional};
            break;
            default:
            PERROR("invalid entity type, only client and server entities are suitable\n");
            return -1;
        }
        for(Section *s : cfg){
            Section &section = *s;
            for(auto kv : section){
                if(kv.second == "UNSET"){
                    file << "# ";
                }
                file << kv.first << " " << kv.second << std::endl;
            }
        }
        return 0;
    }
    /* Set the values in templates.conf to the ones in the Section's values, used to save file changes */
    static int sync(){
        std::ifstream conf_file(config_path);
        if(!conf_file.is_open()){
            PERROR("couldn't open config file '{}'\n",config_path);
            return -1;
        }
        str tmp = config_path + ".tmp";
        std::ofstream tmpfile(tmp);
        if(!tmpfile.is_open()){
            PERROR("couldn't create temporary file '{}' failed to synchronize\n", tmp);
            return -1;
        }
        str line, key, val;
        Section *ptr = nullptr;
        char firstchar;
        while(getline(conf_file,line)){
            firstchar = line[0];
            if(firstchar == '#' | firstchar == '[' | line.empty()){
                tmpfile << line << std::endl;;
                continue;
            }
            sstream ss(line);
            ss >> key;
            ptr = VpnConfig::prop_exists(key);
            if(ptr == nullptr){
                PWARN("skipping unknown property '{}'\n",key);
                continue;
            }
            strview newval = (*ptr)[key];
            tmpfile << key << " " << newval << std::endl;
        }
        PINFO("Synchronizing vpn config\n");
        if(!fs::remove(config_path)){   
            PERROR("couldn't remove config file\n");
            fs::remove(tmp);
        };
        fs::rename(tmp,config_path);
        return 0;                                           
    }
    /* Must be called before further stuff is done with this class, this loads config
        from profile's templates.conf file into Sections (std::unordered_map<std::string,std::string>) */
    static int set(Profile &profile){
        if(!empty()){
            clear();
        }
        str path = profile.source + SLASH + template_filename;
        config_path = path;
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
                Section &ref = *section_mapping[line];
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
            Section &ref = *section_mapping[SECTION_CLIENT];
            for(auto &kv : ref){
                if(kv.second == "UNSET"){
                    rcode = -1;
                    PWARN("mandatory field '{}' not set\n", kv.first);
                }
            }
        }else if(type == ET_SV){
            Section &ref = *section_mapping[SECTION_SERVER];
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

    private:
    static inline str config_path;
    static inline std::unordered_map<str, Section*>section_mapping{
        {SECTION_COMMON,&VpnConfig::common},
        {SECTION_CLIENT,&VpnConfig::client},
        {SECTION_CLIENT_OPT,&VpnConfig::client_optional},
        {SECTION_SERVER,&VpnConfig::server},
        {SECTION_SERVER_OPT,&VpnConfig::server_optional}
    };

    static inline bool is_valid_section(str &line){
        return section_mapping.find(line) != section_mapping.end();
    }

    static inline int _set = 0;
    static inline str profile;

    // Returns position to line where next section is
    static int load_section(std::ifstream &file, Section &section, int allow_empty_or_none = 1){
        str line, key, val;
        std::streampos pos;
        while(getline(file,line)){
            sstream ss;
            char first = line[0];
            if(line.empty() || first == '#'){
                // empty / commented line, not interested
                continue;
            }
            if(first == '['){
                // reached new section
                return pos;
            }
            ss << line;
            getline(ss,key,' ');
            getline(ss,val);
            section.emplace(key,val);
            ss.clear();
            pos = file.tellg();
        }
        return 0;
    }
};
} // namespace gpki