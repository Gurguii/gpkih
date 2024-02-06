#include "actions.hpp"
using namespace gpki;
static inline str key_value_separator = " = ";

static inline str styled_title(const char *st){
    return fmt::format(fg(WHITE) | EMPHASIS::bold,st);
}
static inline str styled_key_prop(strview key, strview val){
    return fmt::format(fg(CYAN) | EMPHASIS::bold,"{}",key) + key_value_separator + fmt::format(fg(WHITE)|EMPHASIS::bold| EMPHASIS::italic,"{}",val);
}
int actions::get(subopts::get &params){
    Profile &profile = params.profile;
    if(!params.cl_properties.empty()){
        for(auto &prop : params.cl_properties){
            if(VpnConfig::client.find(prop.data()) != VpnConfig::client.end()){
                PRINTF("{}.{}\n",styled_title("client"),styled_key_prop(prop, VpnConfig::client[prop.data()]));
            }else{
                PRINTF("{}.{}\n",styled_title("client"),styled_key_prop(prop, VpnConfig::client_optional[prop.data()]));
            }
        }
    }
    if(!params.sv_properties.empty()){
        for(auto &prop : params.sv_properties){
            if(VpnConfig::server.find(prop.data()) != VpnConfig::server.end()){
                PRINTF("{}.{}\n",styled_title("server"),styled_key_prop(prop, VpnConfig::server[prop.data()]));
            }else{
                PRINTF("{}.{}\n",styled_title("server"),styled_key_prop(prop, VpnConfig::server_optional[prop.data()]));
            }
        }
    }
    if(!params.common_properties.empty()){
        for(auto &prop : params.common_properties){
            PRINTF("{}.{}\n",styled_title("common"),styled_key_prop(prop, VpnConfig::common[prop.data()]));
        }
    }
    // ./gpki get <profile> prop1,prop2...propN
    // ./gpki get test client.
    return 0;
}