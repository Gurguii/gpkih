#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#ifdef __WIN32
#define EOL std::string("\r\n")
#else
#define EOL std::string("\n")
#endif

#define dbpath std::string("profiles.db")
#define dbheaders std::string("id,name,source,ca" + EOL)

struct Profile {
  int id;
  std::string name;
  std::string source;
  int ca;
};
namespace gpki::db {
static int populate_entry(std::string entry, Profile *profile) {
  // entry e.g 1,wiski,/home/gurgui/test,0
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return -1;
  }
  std::string line;
  std::string commas;
  while (getline(file, line)) {
    std::stringstream ss(line);
    ss >> profile->id;
    ss >> commas;
    ss >> profile->name;
    ss >> commas;
    ss >> profile->source;
    ss >> commas;
    ss >> profile->ca;
  }
  return 0;
}
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
  std::ifstream(dbpath).read(&headers[0], headers.size());
  std::cout << "headers -> " << headers << "\ndbheaders -> " << dbheaders
            << "\n";
  std::cout << headers.size() << " " << dbheaders.size() << "\n";
  if (headers != dbheaders) {
    std::cout << "headers do not match\n";
    return -1;
  }
  std::cout << "database exists, all ok\n";
  return 0;
} // namespace gpki::db
} // namespace gpki::db

namespace gpki::db::profiles {

static int exists(Profile *profile) {
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    std::cout << "couldn't open database\n";
    return -1;
  }
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_entry(line, &entry);
    std::cout << "entry name: " << entry.name
              << " profile name: " << profile->name << "\n";
    if (entry.name == profile->name || entry.id == profile->id) {
      file.close();
      return 1;
    }
  }
  file.close();
  return 0;
}
static int add(Profile *profile) {
  if (exists(profile)) {
    return -1;
  }
  int s = std::filesystem::file_size(dbpath);
  std::string entry = std::to_string(profile->id) + " , " + profile->name +
                      " , " + profile->source + " , " +
                      std::to_string(profile->ca) + EOL;
  std::ofstream db(dbpath, std::ios::app);
  db << entry;
  return (std::filesystem::file_size(dbpath) > s ? 0 : -1);
}
static int del(Profile *profile) {
  int s = std::filesystem::file_size(dbpath);
  std::ifstream file(dbpath);
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_entry(line, &entry);
  }
  return 0;
}
} // namespace gpki::db::profiles
using namespace gpki;
int main() {
  if (gpki::db::initialize()) {
    std::cout << "couldn't initialize database\n";
    return -1;
  };
  Profile t{.id = 1, .name = "test", .source = "/home/test", .ca = 1};
  db::profiles::add(&t);
  return 0;
}
