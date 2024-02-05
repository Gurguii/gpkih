#include "actions.hpp"
using namespace gpki;

int actions::get(subopts::get &params){
    Profile &profile = params.profile;
    if(!params.cl_properties.empty()){
        PRINT("== client ==\n",S_INFO);
        for(auto &prop : params.cl_properties){
            if(VpnConfig::client.find(prop.data()) != VpnConfig::client.end()){
                PRINT(VpnConfig::client[prop.data()],S_NONE);
            }else{
                PRINT(VpnConfig::client_optional[prop.data()],S_NONE);
            }
        }
    }
    if(!params.sv_properties.empty()){
        PRINT("== server ==\n",S_INFO);
        for(auto &prop : params.sv_properties){
            if(VpnConfig::server.find(prop.data()) != VpnConfig::server.end()){
                PRINT(VpnConfig::server[prop.data()],S_NONE);
            }else{
                PRINT(VpnConfig::server_optional[prop.data()],S_NONE);
            }
        }
    }
    if(!params.common_properties.empty()){
        PRINT("== common ==\n", S_INFO);
        for(auto &prop : params.common_properties){
            PRINT(VpnConfig::common[prop.data()], S_NONE);
        }
    }
    // ./gpki get <profile> prop1,prop2...propN
    // ./gpki get test client.
    return 0;
}