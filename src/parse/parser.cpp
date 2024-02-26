#include "parser.hpp"

using namespace gpkih;
// [!] parse() does not expect to receive program name in args
int parsers::parse(int argc, const char **_args) {
  if (argc == 0) {
    help::usage_brief();
    // TODO - add a gpkih cli mode where the call to gpkih 
    // can be omitted, something like 'virsh'
    return GPKIH_OK;
  }
  std::vector<str> args(_args, _args + argc);

  // Check global config options
  for (int i = 0; i < argc; ++i) {
    strview op = args[i];
    if (op == "-y") {
      Config::set("behaviour","autoanswer","yes");
      args.erase(args.begin() + i);
      --argc;
    } else if (op == "--noprompt") {
      Config::set("behaviour","prompt","no");
      args.erase(args.begin() + i);
      --argc;
    }
    else if (op == "--noprint") {
      Config::set("behaviour", "print_generated_certificate", "no");
      args.erase(args.begin() + i);
      --argc;
    }
  }

  // At this point action is mandatory
  if (args.size() == 0) {
    help::usage();
    return GPKIH_OK;
  }

  str action = args[0];

  if (action == "help" || action == "--help" || action == "-help" ||
      action == "-h") {
    if (args.size() > 1) {
      //fmt::print("call_helper() - parse/parser.cpp - line 39\n");
      help::call_helper(args[1]);
    } else {
      help::usage();
    }
    return GPKIH_OK;
  }

  // Check if action exists
  if (ACTION_PARSERS.find(action) == ACTION_PARSERS.end()) {
    PERROR("action '{}' doesn't exist\n", action);
    return -1;
  }
  
  args.erase(args.begin());
  return ACTION_PARSERS[action](std::move(args));
}
