#include "gpki.hpp"
#include "config_management.cpp"
#include "printing.hpp"
#include "structs.hpp"
/* Parser & subparsers */
#include "parse/parser.cpp"
/* Database (csv) manipulation */
#include "db/entities.cpp"
#include "db/profiles.cpp"
/* Available actions */
#include "actions/actions.cpp"
/* Help functions */
#include "help/help.cpp"

// Tasks launched by cleanup() before exiting the program
static inline std::vector<int (*)()> cleanup_functions{db::profiles::sync};
static inline void cleanup() {
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
  static inline void ctrl_c_handler(int sig) {
    PINFO("\ncaught ctrl+c signal, cleaning up before exiting ...");
    cleanup();
  }

public:
  static inline void register_signals() {
#ifdef __WIN32
    SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
#else
    signal(SIGINT, ctrl_c_handler);
#endif
  }
};

int check_gpkih_install_dir() {
  if (!fs::exists(BASEDIR)) {
    if (!fs::create_directory(BASEDIR)) {
      PERROR("Couldn't create directory '{}'", BASEDIR);
      return F_NOEXIST;
    };
    str configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
    fs::copy(configdir, CONF_DIRPATH, fs::copy_options::recursive);
    if (!fs::exists(CONF_DIRPATH) || !fs::is_directory(CONF_DIRPATH)) {
      PERROR("couldn't create gpkih source dir '{}'\n", CONF_DIRPATH);
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
  PROGRAMSTARTING();

  // Launch task to load gpkih config
  auto load_gpkih_config = std::async(std::launch::async, gpkih::Config::load);

  // Map profiles' csv to db::profiles::existing_profiles{}
  int profile_count = -1;
  if ((profile_count = db::profiles::initialize()) == -1) {
    cleanup();
  };

  // wait for task
  if (load_gpkih_config.get() != GPKIH_OK) {
    PERROR(lasterror());
    return -1;
  }

  PINFO("Loaded [{}] profiles\n", profile_count, DB_DIRPATH, SLASH);

  // Parse options
  if (gpkih::parsers::parse(argc - 1, args + 1) != GPKIH_OK) {
    PERROR(lasterror());
    cleanup();
  }

  // Everything went well
  cleanup();
}
