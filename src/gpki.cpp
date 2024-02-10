#include "printing.hpp"
#include "structs.hpp"
#include "gpki.hpp"
/* Parser & subparsers */
#include "parse/parser.cpp"
/* Database (csv) manipulation */
#include "db/entities.cpp"
#include "db/profiles.cpp"
/* Available actions */
#include "actions/actions.cpp"
/* Help functions */
#include "help/help.cpp"

static inline void ctrl_c_handler(int sig){
  std::cout << "\n";
  PROGRAMEXITING();
  PINFO("cleaning up before exiting ...\n\n");
  /* do cleanup */
  exit(0);
}

static inline void register_signals(){
  #ifdef __WIN32
  SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
  #else
  signal(SIGINT,ctrl_c_handler);
  #endif
}

int main(int argc, const char **args) {
  // Register signals
  register_signals();
  // Add PROPER checks for openssl - openvpn existence
  PROGRAMSTARTING();
  if (!fs::exists(BASEDIR)) {
    if (!fs::create_directory(BASEDIR)) {
      PERROR("Couldn't create directory '{}'",BASEDIR);
      return -1; 
    };
    str configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
    fs::copy(configdir, CONF_DIRPATH, fs::copy_options::recursive);
    if (!fs::exists(CONF_DIRPATH) || !fs::is_directory(CONF_DIRPATH)) {
      PERROR("couldn't create gpkih source dir '{}'\n",CONF_DIRPATH);
      return -1;
    }
    if (!fs::create_directory(DB_DIRPATH)) {
      PERROR("couldn't create gpkih source db dir '{}'\n",DB_DIRPATH);
      return -1;
    };
  }
  int p = db::profiles::initialize();
  if(p == -1){
    return -1; 
  }
  PINFO("Loaded [{}] profiles\n",p,DB_DIRPATH,SLASH);
  if (gpki::parsers::parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
