#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <map>
#include <vector>
#include "../../src/utils/gpkih_util_funcs.hpp"
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
    const char *path;
    std::unordered_map<str,str>sed_map{};
    protected:
    static inline Config conf{
        {"proto",""},
        {"dev",""},
        {"cipher",""}
    };

    public:
    void print(){
        for(const auto &kv : conf){
            std::cout << "key: " << kv.first << " value: " << kv.second << "\n";
        }
    }
    Config get(){
        return conf;
    }
    void load(){
        std::ifstream file(path);
        if(!file.is_open()){
            std::cerr << "couldn't open file " << path << "\n";
            return;
        }
        str line;
        str key, val;
        sstream ss;
        while(getline(file,line)){
            if(line[0] == '#' || line[0] == '[' || line.empty()){
                continue;
            }
            ss.write(line.c_str(),line.size());
            getline(ss,key,' ');
            getline(ss,val,'\n');
            if(conf.find(key) != conf.end()){
                conf[key] = val;
            }
            ss.clear();
        }
    }
    void clear(){
        conf.clear();
    }
    vpn_config(const char *_path, ETYPE type):path(_path){
        switch(type){
            case ETYPE::cl:
                conf.insert({
                {"remote",""},
                {"remote-cert-tls",""},
                {"tls-auth",""}});
                break;
            case ETYPE::sv:
                conf.insert({
                {"server",""},
                {"port",""},
                {"status",""},
                {"explicit-exit-notify",""},
                {"tls-auth",""},
                       {"ifconfig-pool-persist",""}});
                break;
            default:
                std::cerr << "Not a valid type\n";
                break;
        }
    };
};

int main()
{
    vpn_config configuration("templates.conf",ETYPE::sv);
    configuration.load();
    configuration.print();
    Config mapped = configuration.get();
    std::unordered_map<strview,strview> mm;
    mm.insert(mapped.begin(),mapped.end());
    sed("server.ovpn","server_changed.ovpn",mm);
    return 0;
}