#include "database.hpp"
#include <sstream>
#include <fstream> // std::ifstream | std::ofstream
#include <iostream> // std::cin
using namespace gpkih;

int db::profiles::populate_from_entry(str &entry, Profile *profile) {
  sstream ss(entry);
  std::getline(ss, profile->name, ',');
  std::getline(ss, profile->source, ',');
  return 0;
}

int db::profiles::populate_from_entry(str &entry, std::vector<str> &fields) {
  str token;
  sstream ss(entry);
  while (getline(ss, token, ',')) {
    fields.push_back(token);
  }
  return 0;
}

/* Synchronizes profiles.csv with existing_profiles so that only valid profiles
(with existing source dir) in existing_profiles are left in profiles.csv */
int db::profiles::sync() {
  PINFO("synchronizing profiles\n");
  std::string tmpfile = dbpath + ".tmp";
  std::ofstream tmp(tmpfile);
  if (!tmp.is_open()) {
    seterror("couldn't open tmpfile to synchronize database\n");
    PERROR("couldn't open tmpfile to synchronize database\n");
    return -1;
  }
  tmp.write(&dbheaders[0],dbheaders.size());
  for (auto &p : existing_profiles) {
    Profile &ref = p.second;
    if (std::filesystem::exists(ref.source) &&
        std::filesystem::is_directory(ref.source)) {
      // good
      tmp << EOL << (ref.csv_entry());
    }
  }
  tmp.close();
  std::filesystem::remove(dbpath);
  std::filesystem::rename(tmpfile, dbpath);
  // PINFO("Database synchronized\n");
  return GPKIH_OK;
}
/* Requires only 1 call since the profiles
 * are all in the same csv */
size_t db::profiles::initialize(strview path) {
  dbpath = path;
  if (!fs::exists(dbpath)) {
    // Create
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      PERROR("couldn't create file '{}'\n", dbpath);
      return -1;
    }
    db.write(&dbheaders[0], dbheaders.size());
    db << EOL;
    // db << dbheaders;
    db.close();
    return GPKIH_OK;
  }
  std::ifstream file(dbpath);
  str headers("\0",dbheaders.size());
  file.read(&headers[0], headers.size());
  if (headers != dbheaders) {
      seterror("profile headers do not match - original: '{}' size: '{}' current: '{}' size: '{}'",
          dbheaders, dbheaders.size(), headers, headers.size());
    file.close();
    return GPKIH_FAIL;
  }
  // Load profiles into existing_profiles
  std::vector<str> remove_profiles;
  str line;
  int lines = 0;
  while (getline(file, line)) {
      if (line.empty()) {
          continue;
      }
    Profile pinfo;
    ++lines;
    populate_from_entry(line, &pinfo);
    if (!fs::exists(pinfo.source)) {
      PWARN("Profile '{}' is in the database but source dir '{}' doesn't "
            "exist, removing from db\n",
            pinfo.name, pinfo.source);
      // profile must be in existing_profiles since remove() checks for it
      remove_profiles.emplace_back(pinfo.name);
    }
    // emplace() returns pair<iterator,bool> where the bool indicates if
    // emplacement was sucessful or not
    if (!existing_profiles.emplace(pinfo.name, pinfo).second) {
      PERROR("couldn't add profile '{}' to existing_profiles\n", pinfo.name);
      return -1;
    };
  }
  file.close();
  if (remove_profiles.size() > 0) {
      // found entries whose source dirs don't exist
      // remove them
      db::profiles::remove(remove_profiles);
  }
  return existing_profiles.size();
}

int db::profiles::exists(strview profile_name) {
  return existing_profiles.find(profile_name.data()) != existing_profiles.end();
}

int db::profiles::add(Profile *profile) {
  if (exists(profile->name)) {
    seterror("error adding profile '{}' to database - already exists", profile->name);
    return GPKIH_FAIL;
  }
  size_t bsize = fs::file_size(dbpath);
  std::ofstream db(dbpath, std::ios::app);
  // fmt::print("adding profile entry -> {} to file '{}'\n", profile->csv_entry(), dbpath);
  if(!db.is_open()){
    seterror("couldn't open db file '{}'", dbpath);
    return F_NOOPEN;
  }
  db << profile->csv_entry() << EOL;
  db.close();

  if (fs::file_size(dbpath) <= bsize) {
    seterror("error adding profile '{}' to database - file size didn't change after adding entry\n", profile->name);
    return GPKIH_FAIL;
  };
  return (existing_profiles.emplace(profile->name, *profile).second) 
          ? GPKIH_OK 
          : GPKIH_FAIL;
}

int db::profiles::remove(std::vector<str> &profiles) {
  for (auto &profile : profiles) {
    auto iter = existing_profiles.find(profile);
    if (iter == existing_profiles.end()) {
      return -1;
    }
    Profile target = iter->second;
    if (Config::get("behaviour","autoanswer") == "no") {
      // ask before removing
      str ans;
      PROMPT("Files from profile '" + profile +
                 "' about to get removed, continue?",
             "[y/n]:");
      std::getline(std::cin, ans);
      if (ans != "y" && ans != "Y") {
        PINFO("not removing anything\n");
        return 0;
      }
    }
    // remove profile files
    if (fs::exists(target.source)) {
      if (!std::filesystem::remove_all(target.source)) {
        seterror("couldn't remove source dir for profile '" + target.name +
                 "'");
        return -1;
      };
    }
    // remove profile from existing_profiles
    existing_profiles.erase(iter);
    // remove profile entities db
    str db = gpkih::db::profiles::dbpath + target.name + "_entities.csv";
    if (fs::exists(db)) {
      if (!fs::remove(db)) {
        seterror(fmt::format("couldn't remove entities' csv '{}'", db));
        return -1;
      }
    }
  }
  // call sync() to synchronize existing_profiles with profiles.csv contents
  return sync();
}

int db::profiles::remove_all() {
  // Using the map itself would cause in segmentation fault since
  // remove() calls existing_profiles.erase()
  std::vector<str> profiles;
  for (auto kv : existing_profiles) {
    profiles.emplace_back(kv.first);
  }
  remove(profiles);
  return 0;
}

int db::profiles::load(strview profile_name, Profile &pinfo) {
  if (!exists(profile_name)) {
    seterror("profile '{}' doesn't exist\n",profile_name);
    return GPKIH_FAIL;
  }
  pinfo = existing_profiles[profile_name.data()];
  return GPKIH_OK;
}

Profile *const db::profiles::load(strview profile_name) {
  if (!exists(profile_name)) {
    seterror("profile '{}' doesn't exist\n",profile_name);
    return nullptr;
  }
  return &existing_profiles[profile_name.data()];
}

int db::profiles::get_entities(str profile, std::vector<Entity> &buff) {
  str edb = gpkih::db::profiles::dbpath + profile + "_entities.csv";
  std::ifstream file(edb);
  if (!file.is_open()) {
    PERROR("couldn't open database {}\n", edb);
    return -1;
  }
  std::string entry;
  // avoid first line which are the headers
  getline(file, entry);
  entry.assign("");
  while (getline(file, entry)) {
    Entity entity;
    entities::populate_from_entry(entry, entity);
    buff.push_back(std::move(entity));
  }
  file.close();
  return GPKIH_OK;
}
