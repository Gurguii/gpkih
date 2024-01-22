#pragma once
#include <iostream>
#include <unordered_map>


namespace gpki::help::generic {
void usage();
}
namespace gpki::help::build {
void usage();
}
namespace gpki::help::revoke {
void usage();
}
namespace gpki::help::gencrl{
void usage();
}
namespace gpki::help::list{
void usage();
}
namespace gpki::help::init{
void usage();
}

using namespace gpki;
void call_helper(std::string &action){
  if(action == "build"){
    help::build::usage();
  }else if(action == "revoke"){
    help::revoke::usage(); 
  }else if(action == "init"){
    help::init::usage();
  }else if(action == "gencrl"){
    help::gencrl::usage();
  }else if(action == "list"){
    help::list::usage();
  }
}
