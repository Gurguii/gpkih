#include "subparser.hpp"

struct build_params
{
  std::string key_size = "1024";
  std::string in_format = "pem";
  std::string out_format = "pem";
};
// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpki;
int subparsers::build(std::vector<std::string> opts) { 
  std::string_view profile = opts[0];
  int (*function_to_call)() = nullptr;
  if(!db::profiles::exists(profile)){
    std::cout << "profile '" << profile << "' doesn't exist\n";
    return -1;
  }
  opts.erase(opts.begin(),opts.begin()+1);
  opts.push_back("\0");
  build_params params;
  // override default build params with user arguments and set
  // right function to be executed
  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    std::cout << "opt -> " << opt << "\n";
    if(opt == "-ca"){
      function_to_call = modes::build::ca;
    }else if(opt == "-sv" || opt == "-server"){
      function_to_call = modes::build::server;
    }else if(opt == "-cl" || opt == "-client"){
      function_to_call = modes::build::client;
    }else if(opt == "-keysize" || opt == "--keysize"){
      params.key_size = opt[++i];
    }else if(opt == "-informat"){
      params.in_format = opt[++i];
    }else if(opt == "-outformat"){
      params.out_format = opt[++i];
    }
  }
  return function_to_call();
}
