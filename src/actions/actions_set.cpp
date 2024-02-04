#include "actions.hpp"
int actions::set(subopts::set &params){
    Section all;
    all.insert(VpnConfig::common.begin(),VpnConfig::common.end());
    all.insert(VpnConfig::client.begin(),VpnConfig::client.end());
    all.insert(VpnConfig::server.begin(),VpnConfig::server.end());
    return 0;
}