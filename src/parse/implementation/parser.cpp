#include "../parser.hpp"
#include "../../help/help.hpp"
#include "../../cli/cli.hpp"
#include "../../config/Config.hpp"

std::unordered_map<std::string, int (*)(std::vector<std::string> &opts)> ACTION_PARSERS{
    {"init", gpkih::parsers::init},
    {"i", gpkih::parsers::init},

    {"list", gpkih::parsers::list},
    {"l", gpkih::parsers::list},

    {"build", gpkih::parsers::build},
    {"b", gpkih::parsers::build},

    {"revoke", gpkih::parsers::revoke},
    {"r", gpkih::parsers::revoke},

    {"gencrl", gpkih::parsers::gencrl},

    {"remove", gpkih::parsers::remove},
    {"rm", gpkih::parsers::remove},

    {"genkey", gpkih::parsers::genkey},
    {"gk", gpkih::parsers::genkey},

    {"get", gpkih::parsers::get},
    {"g", gpkih::parsers::get}, // Conflict with "gencrl"

    {"set", gpkih::parsers::set},
    {"s", gpkih::parsers::set},

    {"rename", gpkih::parsers::rename},
    {"rn", gpkih::parsers::rename},

    {"reset", gpkih::parsers::reset},
    {"rs", gpkih::parsers::reset},

    {"export", gpkih::parsers::dbexport},
    {"e", gpkih::parsers::dbexport}
};

using namespace gpkih;

int parsers::parseGlobals(std::vector<std::string> &opts){
  for(int i = 0; i < opts.size(); ++i){
      if(opts[i] == "-debug" || opts[i] == "--debug"){
          #ifndef GPRINTING_ENABLE_DEBUGGING
          PWARN("This version of gpkih wasn't compiled with debugging capabilities\n");
          PHINT("For such thing, you can compile gpkih using the setup script: ./setup -ed OR ./setup -d GPRINTING_ENABLE_DEBUGGING=ON\n");
          #endif
  
          if(i+1 == opts.size()){
            #ifndef GPRINTING_ENABLE_DEBUGGING
            opts.erase(opts.begin()+i);
            break;
            #endif
            PERROR("Debug level must be given\n");
            return GPKIH_FAIL;
          }
  
          int debugLevel = std::stoi(opts[i+1]);
  
          if(debugLevel > 0 && debugLevel <= 3){
            ENABLE_DEBUG_MESSAGES = true;
            MAX_DEBUG_LEVEL = debugLevel;
            opts.erase(opts.begin()+i,opts.begin()+i+2);
            ++i;
            break;
          }
          #ifdef GPRINTING_ENABLE_DEBUGGING
          PERROR("Debug level must be 0-3 (both included)\n");
          return GPKIH_FAIL;
          #endif
      }
  }
  for(int i = 0; i < opts.size();){
      std::string_view opt = opts[i];
      if(opt == "-q" || opt == "--quiet"){
          ENABLE_PRINTING = false;
          opts.erase(opts.begin()+i);
      }else if(opt == "-dr" || opt == "--dryrun"){
          DRY_RUN = true;
          opts.erase(opts.begin()+i);
      }else if(opt == "-nh" || opt == "--noheader"){
          SHOW_HEADER = false;
          opts.erase(opts.begin()+i);
      }else{
          ++i;    
      }
  }
  return GPKIH_OK;
}

// [!] parse() does not expect to receive program name in args
int parsers::parse(std::vector<std::string> &args) {
  DEBUG(1,"parsers::parse()");

  if (args.size() == 0) {
    help::usage_brief();
    return GPKIH_OK;
  }

  // Override global config options
  for (int i = 0; i < args.size();) {
    std::string_view op = args[i];
    if (op == "-y" || op == "--yes") {
      if(Config::set("behaviour","autoanswer","yes") == GPKIH_FAIL){
        PWARN("couldn't set '{}' opt", op);
      };
      args.erase(args.begin() + i);
    } else if (op == "-noprompt" || op == "--noprompt") {
      if(Config::set("behaviour","prompt","no") == GPKIH_FAIL){
        PWARN("couldn't set '{}' opt", op);
      };
      args.erase(args.begin() + i);
    }else if (op == "-noprint" || op == "--noprint") {
      if(Config::set("behaviour","print_generated_certificate", "no") == GPKIH_FAIL){
        PWARN("couldn't set '{}' opt", op);
      };
      args.erase(args.begin() + i);
    }else{
      ++i;
    }
  }

  // At this point action is mandatory
  if (args.size() == 0) {
    help::usage_brief();
    return GPKIH_OK;
  }

  std::string action = args[0];

  if(action == "cli"){
    gpkih::cli::init();
    return GPKIH_OK;
  }

  // Check if user is requesting some sort of help
  if (action == "help" || action == "--help" || action == "-help" || action == "-h") {
    if(args.size() > 1){
      if(help::helpers.find(args[1]) == help::helpers.end()){
        PERROR("Help for action '{}' doesn't exist\n", args[1]);
        return GPKIH_FAIL;
      }
      if(args.size() > 2){
        std::string_view subHelp = args[2];
        if(subHelp == "examples"){  
          fmt::print("{}",help::helpers[args[1]].examples);
        }else{
          PERROR("Unexistant help sub-option {}", subHelp);
        }
      }else{
        fmt::print("{}", help::helpers[args[1]].usage);
      }
    }else{
      help::usage();
    }
    return GPKIH_OK;
  }

  // Check if action exists
  if (ACTION_PARSERS.find(action) == ACTION_PARSERS.end()) {
    PERROR("Action '{}' doesn't exist\n", action);
    return GPKIH_FAIL;
  }
  
  args.erase(args.begin());
  
  return ACTION_PARSERS[action](args);
}