#include "parser.hpp"

#include "../help/help.hpp"
#include "../cli/cli.hpp"
#include "../libs/printing/printing.hpp"
#include "../config/Config.hpp"
#include "../gpkih.hpp"

#include "../actions/actions.hpp" // getAction

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
    
    if(args.size() == 1){
      help::usage();
      return GPKIH_OK;
    }

    std::string _a = args[1];
    args.erase(args.begin(), args.begin()+2);

    auto a = actions::GetAction(_a);

    if(a == nullptr){
      PERROR("Action '{}' doesn't exist\n", _a);
    }

    a->help(args);

    return GPKIH_OK;
  }

  args.erase(args.begin());
  // Check if action exists
  auto a = actions::GetAction(action);  

  if(a != nullptr){
    return a->exec(args);
  }

  PERROR("Action '{}' doesn't exist\n", action);
  return GPKIH_FAIL;
}