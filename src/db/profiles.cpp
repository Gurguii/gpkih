#include "database.hpp"
using namespace gpki;

int db::profiles::populate_from_entry(str &entry, Profile *profile) {
  sstream ss(entry);
  std::getline(ss, profile->name, CSV_DELIMITER_c);
  std::getline(ss, profile->source, CSV_DELIMITER_c);
  return 0;
}

int db::profiles::populate_from_entry(str &entry,
                                      std::vector<str> &fields) {
  str token;
  sstream ss(entry);
  while (getline(ss, token, ',')) {
    fields.push_back(token);
  }
  return 0;
}

/* Requires only 1 call since the profiles
* are all in the same csv */
int db::profiles::initialize() {
  if (!fs::exists(dbpath)) {
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
  str headers;
  getline(file, headers);
  if (headers != dbheaders) {
    std::cout << "profile headers do not match\n";
    return -1;
  }
  // Load profiles into existing_profiles
  str line;
  Profile pinfo;
  while (getline(file, line)) {
    populate_from_entry(line, &pinfo);
    existing_profiles.emplace(pinfo.name, pinfo);
  }
  return 0;
}

int db::profiles::exists(strview profile_name) {
  return existing_profiles.find(profile_name.data()) != existing_profiles.end();
}

int db::profiles::add(Profile *profile) {
  if (exists(profile->name)) {
    return -1;
  }
  int bsize = fs::file_size(dbpath);
  std::ofstream db(dbpath, std::ios::app);
  db << profile->csv_entry() << std::endl;
  return (fs::file_size(dbpath) > bsize ? 0 : -1);
}

int db::profiles::del(strview profile) {
  int s = fs::file_size(dbpath);
  std::ifstream file(dbpath);
  str line;
  Profile entry;
  while (getline(file, line)) {
    populate_from_entry(line, &entry);
  }
  return 0;
}

int db::profiles::load(strview profile_name, Profile &pinfo) {
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

int db::profiles::get_fields(str profile, std::vector<str> &buff){
  buff.push_back(existing_profiles[profile].name);
  buff.push_back(existing_profiles[profile].source);
  return 0;
}
