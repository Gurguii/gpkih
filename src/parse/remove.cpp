#include "parser.hpp"

using namespace gpki;

int parsers::remove(std::vector<std::string> opts){
    /* Parse subopts */
    if(opts.empty()){
        PERROR("profile name must be given\n");
        PHINT("try gpki help remove\n");
        return -1;
    }
    subopts::remove params;
    std::vector<str> profilenames;
    sstream names(opts[0]);
    str profile;
    while(getline(names,profile,CSV_DELIMITER_c)){
        if(!db::profiles::exists(profile)){
            PWARN("Profile '{}' doesn't exist, omitting\n", profile);
            continue;
        }
        profilenames.emplace_back(profile);
    }
    return db::profiles::remove(profilenames);
}