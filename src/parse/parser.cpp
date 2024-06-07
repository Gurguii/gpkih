#include "parser.hpp"
#include "../help/help.hpp"
#include "../cli/cli.hpp"

using namespace gpkih;

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
    help::usage();
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
      help::call_helper(args[1]);
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