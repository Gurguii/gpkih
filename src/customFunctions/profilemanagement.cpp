#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#ifdef __WIN32
#define EOL std::string("\n")
#else
#define EOL std::string("\r\n")
#endif

#define dbpath std::string("profiles.db")
#define dbheaders std::string("id,name,source" + EOL)

namespace gpki::db {
static int initialize() {
  if (!std::filesystem::exists(dbpath)) {
    // Create
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      std::cout << "couldn't create file '" << dbpath << "'\n";
      return -1;
    }
    // db << dbheaders << EOL;
    db << dbheaders;
    db.close();
    std::cout << "database created\n";
    return 0;
  }
  std::string headers(dbheaders.size(), '\x00');
  std::ifstream file(dbpath);
  std::cout << "headers -> " << headers << "\ndbheaders -> " << dbheaders
            << "\n";
  std::cout << headers.size() << " " << dbheaders.size() << "\n";
  if (strcmp(headers.c_str(), dbheaders.c_str())) {
    std::cout << "headers do not match\n";
    return -1;
  }
  std::cout << "database exists, all ok\n";
  return 0;
} // namespace gpki::db
} // namespace gpki::db
int main() {
  if (gpki::db::initialize()) {
    std::cout << "couldn't initialize database\n";
  };
  return 0;
}
