#include "database.hpp"
using namespace gpki;

int db::profiles::populate_from_entry(std::string &entry, Profile *profile) {
  std::stringstream ss(entry);
  std::getline(ss, profile->name, CSV_DELIMITER_c);
  std::getline(ss, profile->source, CSV_DELIMITER_c);
  return 0;
}
int db::profiles::populate_from_entry(std::string &entry,
                                      std::vector<std::string> &fields) {
  std::string token;
  std::stringstream ss(entry);
  while (getline(ss, token, ',')) {
    fields.push_back(token);
  }
  return 0;
}
int db::profiles::initialize() {
  if (!std::filesystem::exists(dbpath)) {
    // Create
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      std::cout << "couldn't create file '" << dbpath << "'\n";
      return -1;
    }
    db << dbheaders << std::endl;
    db.close();
    // std::cout << "database created\n";
    return 0;
  }
  std::ifstream file(dbpath);
  std::string headers;
  getline(file, headers);
  if (headers != dbheaders) {
    std::cout << "profile headers do not match\n";
    return -1;
  }
  // Load profiles into existing_profiles
  std::string line;
  Profile pinfo;
  while (getline(file, line)) {
    populate_from_entry(line, &pinfo);
    existing_profiles.emplace(pinfo.name, pinfo);
  }
  return 0;
}

int db::profiles::exists(std::string_view profile_name) {
  return existing_profiles.find(profile_name.data()) != existing_profiles.end();
}

int db::profiles::add(Profile *profile) {
  if (exists(profile->name)) {
    return -1;
  }
  int bsize = std::filesystem::file_size(dbpath);
  std::ofstream db(dbpath, std::ios::app);
  db << profile->csv_entry() << std::endl;
  return (std::filesystem::file_size(dbpath) > bsize ? 0 : -1);
}

int db::profiles::del(Profile *profile) {
  int s = std::filesystem::file_size(dbpath);
  std::ifstream file(dbpath);
  std::string line;
  Profile entry;
  while (getline(file, line)) {
    populate_from_entry(line, &entry);
  }
  return 0;
}

int db::profiles::load(std::string_view profile_name, Profile &pinfo) {
  if (!exists(profile_name)) {
    return -1;
  }
  pinfo = existing_profiles[profile_name.data()];
  return 0;
}

int db::profiles::get_entities(
    str profile, std::vector<std::vector<str>> &entities_fields_buff) {
  std::ifstream file(DBDIR + profile + "_entities.csv");
  if (!file.is_open()) {
    return -1;
  }
  str entry;
  std::vector<str> fields;
  while (getline(file, entry)) {
    populate_from_entry(entry, fields);
    entities_fields_buff.push_back(fields);
  }
  return 0;
}
