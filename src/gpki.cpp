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
#include "utils/checkpermissions.cpp"

int main(int argc, const char **args) {
  // Add PROPER checks for openssl - openvpn existence
  PROGRAMSTARTING();
  if (!fs::exists(BASEDIR)) {
    if (!fs::create_directory(BASEDIR)) {
      PRINTF(S_ERROR,"Couldn't create directory '{}'",BASEDIR);
      return -1; 
    };
    str configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
    fs::copy(configdir, CONFDIR, fs::copy_options::recursive);
    if (!fs::exists(CONFDIR) || !fs::is_directory(CONFDIR)) {
      return -1;
    }
    if (!fs::create_directory(DBDIR)) {
      return -1;
    };
  }
  int p = db::profiles::initialize();
  if(p == -1){
    return -1; 
  }
  PINFO("loaded [{}] profiles from {}{}profiles.csv\n",p,DBDIR,SLASH);
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}