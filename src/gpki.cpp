#include "config_management.hpp"
/* Parser & subparsers */
#include "parse/parser.hpp"
/* Database (csv) manipulation */
#include "db/database.hpp"
/* Available actions */
#include "actions/actions.hpp"
/* Help functions */
#include "help/help.hpp"

// Tasks launched by cleanup() before exiting the program
static inline std::vector<int (*)()> cleanup_functions{db::profiles::sync};
static inline int cleanup() {
  std::cout << "\n";
  std::vector<std::future<int>> tasks{};
  // launch every task asynchronously
  for (auto cleanup_func : cleanup_functions) {
    tasks.push_back(std::move(std::async(std::launch::async, cleanup_func)));
  }
  PROGRAMEXITING();
  // wait for every task to finish
  for (auto &t : tasks) {
    t.wait();
  }
  exit(0);
}

class Signals {
private:
#ifdef _WIN32
    static inline BOOL ctrl_c_handler(DWORD signal) {
        PINFO("\ncaught ctrl+c signal, cleaning up before exiting ...");
        cleanup();
        return TRUE;
    }
#else
    static inline void ctrl_c_handler(int sig) {
        PINFO("\ncaught ctrl+c signal, cleaning up before exiting ...");
        cleanup();
    }
#endif
public:
  static inline void register_signals() {
#ifdef _WIN32
    SetConsoleCtrlHandler(ctrl_c_handler, true);
#else
    signal(SIGINT, ctrl_c_handler);
#endif
  }
};

int check_gpkih_install_dir() {
  if (!fs::exists(BASEDIR)) {
      PINFO("Creating gpkih source dir - '{}'", BASEDIR);
    if (!fs::create_directory(BASEDIR)) {
      PERROR("Couldn't create directory '{}'", BASEDIR);
      return F_NOEXIST;
    };
    str configdir = CURRENT_PATH + SLASH + "config";
    fs::copy(configdir, CONF_DIRPATH, fs::copy_options::recursive);
    if (!fs::exists(CONF_DIRPATH) || !fs::is_directory(CONF_DIRPATH)) {
      PERROR("couldn't copy '{}' to '{}'\n", configdir, CONF_DIRPATH);
      return F_NOCREATE;
    }
    if (!fs::create_directory(DB_DIRPATH)) {
      PERROR("couldn't create gpkih source db dir '{}'\n", DB_DIRPATH);
      return F_NOCREATE;
    }; 
  }
  return 0;
}

int main(int argc, const char **args) {
  // Register signal handlers
  Signals::register_signals();

  // Check if gpkih base dir is created, else try to create it
  if (check_gpkih_install_dir() != GPKIH_OK) {
    cleanup();
  }
  // TODO - Add PROPER checks for openssl - openvpn existence

  // Launch task to load gpkih config
  auto load_gpkih_config = std::async(std::launch::async, gpkih::Config::load);

  // Map profiles' csv to db::profiles::existing_profiles{}
  int profile_count = -1;
  if ((profile_count = db::profiles::initialize()) == -1) {
    cleanup();
  };

  // wait for task
  if (load_gpkih_config.get() != GPKIH_OK) {
      printlasterror();
    return -1;
  }
  
  PROGRAMSTARTING();
  
  PINFO("Loaded [{}] profiles\n", profile_count, DB_DIRPATH, SLASH);

  // Parse options
  if (gpkih::parsers::parse(argc - 1, args + 1) != GPKIH_OK) {
      printlasterror();
    cleanup();
  }

  // Everything went well
  cleanup();
}
