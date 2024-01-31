#include "subparser.hpp"

using namespace gpki;

int subparsers::remove(std::vector<std::string> opts){
    /* Parse subopts */
    if(opts.empty()){
        PERROR("profile name must be given\n");
        PINFO("try gpki help remove\n");
        return -1;
    }
    subopts::remove params;
    str profilename = opts[0];
    if(!db::profiles::exists(profilename)){
        PERROR("profile '{}' doesn't exist\n",profilename);
        return -1;
    }
    return db::profiles::remove(profilename);
}