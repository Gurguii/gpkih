#include "parser.hpp"

using namespace gpki;
int parsers::set(std::vector<str> opts){
    // ./gpki set <profile> prop1=val1 ... propN=valN
    // gpki set test client.remote='23.24.25.26 9876'
    if(opts.empty()){
        PERROR("profile name must be given\n");
        PHINT("try gpki help set\n");
        return -1;
    }
    strview profilename = opts[0];
    subopts::set params;
    Profile &profile = params.profile;
    if(db::profiles::load(profilename, profile)){
        PERROR("profile '{}' doesn't exist\n", profilename);
        return -1;
    };
    opts.erase(opts.begin());
    VpnConfig::set(profile);
    str section,prop,nval;
    sstream ss;
    std::vector<str>nonexist_props;

    for(str &properties : opts){
        ss.clear();;
        ss << properties;
        getline(ss,section,'.');
        getline(ss,prop,'=');
        getline(ss,nval);
        if(section == "client" || section == "cl"){
            if(VpnConfig::client_prop_exists(prop)){
                VpnConfig::set_client_prop(prop,nval);
            }
        }else if(section == "server" || section == "sv"){
            if(VpnConfig::server_prop_exists(prop)){
                VpnConfig::set_server_prop(prop,nval);
            }
        }else if(section == "common"){
            if(VpnConfig::common_prop_exists(prop)){
                VpnConfig::set_common_prop(prop,nval);
            }
        }else{
            PERROR("section '{}' doesn't exist\n",section);
            continue;
        }
    }

    //for(auto kv : VpnConfig::client){
    //    std::cout << "key: " << kv.first << "\n";
    //    std::cout << "val: " << kv.second << "\n";
    //}
    
    if(VpnConfig::sync()){
        return -1;
    }
    return 0;
}