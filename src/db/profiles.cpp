#include "database.hpp"
#include <filesystem>
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

/* Synchronizes profiles.csv with existing_profiles so that only valid profiles (with existing source dir) in
existing_profiles are  present in profiles.csv */
int db::profiles::sync(){
  std::string tmpfile = dbpath + ".tmp";
  std::ofstream tmp(tmpfile);
  if(!tmp.is_open()){
    seterror("couldn't open tmpfile to synchronize database\n");
    return -1;
  }
  tmp << dbheaders << std::endl;;
  for(auto &p : existing_profiles){
    Profile &ref = p.second;
    if(std::filesystem::exists(ref.source) && std::filesystem::is_directory(ref.source)){
      // good
      tmp << (ref.csv_entry()) << std::endl;
    }
  }
  std::filesystem::remove(dbpath);
  std::filesystem::rename(tmpfile,dbpath);
  PINFO("Database synchronized\n");
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
  std::vector<str> remove_profiles;
  str line;
  Profile pinfo;
  int lines = 0;
  while (getline(file, line)) {
    ++lines;
    populate_from_entry(line, &pinfo);
    if(!fs::exists(pinfo.source)){
      PWARN("Profile '{}' is in the database but source dir '{}' doesn't exist, removing from db\n",pinfo.name,pinfo.source);
      remove_profiles.emplace_back(pinfo.name);
      continue;
    }
      // emplace() returns pair<iterator,bool> where the bool indicates if emplacement was sucessful or not
      if (!existing_profiles.emplace(pinfo.name,pinfo).second){
        PERROR("couldn't add profile '{}' to existing_profiles\n", pinfo.name);
        return -1;
      };
  }
  db::profiles::remove(remove_profiles);
  return existing_profiles.size(); 
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

int db::profiles::remove(std::vector<str> &profiles) {
  for(auto &profile : profiles){
  
  auto iter = existing_profiles.find(profile);
  if(iter == existing_profiles.end()){
    return -1;
  }
  Profile target = iter->second;
  if(prompt){
    str ans;
    PROMPT("Files from profile '" + profile + "' about to get removed, continue?","[y/n]:");
    std::getline(std::cin,ans);
    if(ans != "y" && ans != "Y"){
      std::cout << "Not removing anything ...\n";
      return 0;
    }
  }
  // remove profile files
  if(!std::filesystem::remove_all(target.source)){
    seterror("couldn't remove source dir for profile '" + target.name + "'");
    return -1;
  };
  // remove profile from existing_profiles
  existing_profiles.erase(iter);
  // remove profile entities db
  str db = DBDIR + target.name + "_entities.csv";
  if(!fs::remove(db)){
    seterror(fmt::format("couldn't remove entities' csv '{}'",db));
    return -1;
  }
  }
  // call sync() to synchronize existing_profiles with profiles.csv contents
  return sync();
}

int db::profiles::remove_all(){
  // Using the map itself would cause in segmentation fault since
  // remove() calls existing_profiles.erase()
  std::vector<str> profiles;
  for(auto kv : existing_profiles){
    profiles.emplace_back(kv.first);
  }
  remove(profiles);
  return 0;
}

int db::profiles::load(strview profile_name, Profile &pinfo) {
  if (!exists(profile_name)) {
    return -1;
  }
  pinfo = existing_profiles[profile_name.data()];
  return 0;
}

int db::profiles::get_entities(str profile, std::vector<Entity> &buff){
  str edb = DBDIR + SLASH + profile + "_entities.csv";
  std::ifstream file(edb);
  if(!file.is_open()){
    PERROR("couldn't open database {}\n",edb);
    return -1;
  }
  std::string entry;
  // avoid first line which are the headers
  getline(file,entry);
  entry.assign("");
  while(getline(file,entry)){
    Entity entity;
    entities::populate_from_entry(entry,entity);
    buff.push_back(std::move(entity));
  }
  return 0;
}