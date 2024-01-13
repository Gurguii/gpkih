#include "database.hpp"
using namespace gpki;
int db::profiles::populate_entry(std::string entry, Profile *profile) {
  // entry e.g 1,wiski,/home/gurgui/test,0
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return -1;
  }
  std::string line;
  std::string commas;
  while (getline(file, line)) {
    std::stringstream ss(line);
    ss >> profile->name;
    ss >> commas;
    ss >> profile->source;
    ss >> commas;
  }
  return 0;
}
int db::profiles::initialize() {
  std::cout << "profile db path -> " << dbpath << "\n";
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
  if (headers != dbheaders) {
    std::cout << "profile headers do not match\n";
    return -1;
  }
  return 0;
}

int db::profiles::exists(Profile *profile) {
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    std::cout << "couldn't open database\n";
    return -1;
  }
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_entry(line, &entry);
    if (entry.name == profile->name) {
      file.close();
      return 1;
    }
  }
  file.close();
  return 0;
}
int db::profiles::exists(std::string_view profile_name) {
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return -1;
  }
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_entry(line, &entry);
    if (entry.name == profile_name) {
      file.close();
      return 1;
    }
  }
  return 0;
}
int db::profiles::add(Profile *profile) {
  if (exists(profile)) {
    return -1;
  }
  int s = std::filesystem::file_size(dbpath);
  std::string entry = profile->name + " , " + profile->source + EOL;
  std::ofstream db(dbpath, std::ios::app);
  db << entry;
  return (std::filesystem::file_size(dbpath) > s ? 0 : -1);
}

int db::profiles::del(Profile *profile) {
  int s = std::filesystem::file_size(dbpath);
  std::ifstream file(dbpath);
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_entry(line, &entry);
  }
  return 0;
}
