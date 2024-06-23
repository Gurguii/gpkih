#include "../parser.hpp"
#include "../../help/help.hpp"
#include "../../cli/cli.hpp"
#include "../../config/Config.hpp"
using namespace gpkih;

int parsers::parseGlobals(std::vector<std::string> &opts){
  for(int i = 0; i < opts.size(); ++i){
      if(opts[i] == "-debug" || opts[i] == "--debug"){
          #ifndef GPKIH_ENABLE_DEBUGGING
          PWARN("This version of gpkih wasn't compiled with debugging capabilities\n");
          PHINT("For such thing, you can compile gpkih using the setup script: ./setup -ed OR ./setup -d GPKIH_ENABLE_DEBUGGING=ON\n");
          #endif
  
          if(i+1 == opts.size()){
            #ifndef GPKIH_ENABLE_DEBUGGING
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
          #ifdef GPKIH_ENABLE_DEBUGGING
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
  PDEBUG(1,"parsers::parse()");

  if (args.size() == 0) {
    help::usage_brief();
    // TODO - add a gpkih cli mode where the call to gpkih 
    // can be omitted, something like 'virsh'
    return GPKIH_OK;
  }

  // Override global config options
  for (int i = 0; i < args.size();) {
    strview op = args[i];
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

  str action = args[0];

  if(action == "cli"){
    gpkih::cli::init();
    return GPKIH_OK;
  }

  // Check if user is requesting some sort of help
  if (action == "help" || action == "--help" || action == "-help" ||
      action == "-h") {
    if (args.size() > 1) {
      help::callHelper(args[1]);
    } else {
      help::usage();
    }
    return GPKIH_OK;
  }

  // Check if action exists
  if (ACTION_PARSERS.find(action) == ACTION_PARSERS.end()) {
    PERROR("action '{}' doesn't exist\n", action);
    return GPKIH_FAIL;
  }
  
  args.erase(args.begin());
  
  return ACTION_PARSERS[action](args);
}