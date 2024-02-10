#include "parser.hpp"
#include "build.cpp"
#include "create_pack.cpp"
#include "gencrl.cpp"
#include "get.cpp"
#include "init.cpp"
#include "list.cpp"
#include "remove.cpp"
#include "revoke.cpp"
#include "set.cpp"

using namespace gpki;
// [!] parse() does not expect to receive program name in args
int parsers::parse(int argc, const char **_args) {
  if (argc == 0) {
    help::usage();
    return -1;
  }
  std::vector<str> args(_args, _args + argc);
  // Check global sub-options 
  for (int i = 0; i < argc; ++i) {
    strview op = args[i];
    if (op == "-y") {
      subopts::params::autoanswer_yes = 1;
      args.erase(args.begin() + i);
      --argc;
    }else if(op == "--noprompt"){
      subopts::params::prompt = 0;
      args.erase(args.begin() + i);
    }
  }
  // Action must be given
  if (args.size() == 0) {
    help::usage();
    return -1;
  }
  str action = args[0];

  if (action == "help") {
    if (args.size() > 1) {
      call_helper(args[1]);
    } else {
      help::usage();
    }
    return 0;
  }
  // Check if action exists
  if (ACTION_PARSERS.find(action) == ACTION_PARSERS.end()) {
    PERROR("action '{}' doesn't exist\n", action);
    return 0;
  }
  args.erase(args.begin());
  return ACTION_PARSERS[action](std::move(args));
}
