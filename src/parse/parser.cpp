#include <cstring>
#include <unordered_map>
#include <vector>

#include "../db/database.hpp"
#include "../help/help.hpp"
#include "../gpki.hpp"

#include "subparser_build.cpp"
#include "subparser_gencrl.cpp"
#include "subparser_init.cpp"
#include "subparser_list.cpp"
#include "subparser_revoke.cpp"
#include "subparser_remove.cpp"
#include "subparser_remove_all.cpp"
#include "subparser_create_pack.cpp"

std::unordered_map<str, int (*)(std::vector<str>)>
    SUBPARSERS{
        {"init",gpki::subparsers::init},
        {"list",gpki::subparsers::list},
        {"remove-all",gpki::subparsers::remove_all},
        {"build", gpki::subparsers::build},
        {"revoke", gpki::subparsers::revoke},
        {"gencrl", gpki::subparsers::gencrl},
        {"remove",gpki::subparsers::remove},
        {"create-pack",gpki::subparsers::create_pack}
    };

namespace gpki {
// [!] parse() does not expect to receive program name in args
int parse(int argc, const char **_args) {
  if (argc == 0) {
    help::usage();
    return -1;
  }
  std::vector<str> args(_args,_args+argc);
  int s = args.size();
  for(int i = 0; i < s; ++i){
    strview op = args[i];
    if (op == "-y" || op == "--noprompt"){
      prompt = 0;
      args.erase(args.begin()+i);
      --s;
    }
  }
  if(args.size() == 0){
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
  if (SUBPARSERS.find(action) == SUBPARSERS.end()){
    PERROR("action '{}' doesn't exist\n",action);
    return 0;
  }
  args.erase(args.begin());
  return SUBPARSERS[action](std::move(args));
}
} // namespace gpki
