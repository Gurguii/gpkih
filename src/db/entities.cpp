#include "database.hpp"
#include <fstream>
using namespace gpki;

/* Requires a call whenever we change the profile context */
int db::entities::initialize(str profile) {
  str dbpath = DBDIR + profile + "_entities.csv";
  if (!fs::exists(dbpath)) {
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      std::cout << "couldn't create file '" << dbpath << "'\n";
      return -1;
    }
    db << dbheaders << std::endl;
    db.close();
    // std::cout << "entity csv created\n";
    return 0;
  }
  str headers(dbheaders.size(), '\x00');
  std::ifstream(dbpath).read(&headers[0], headers.size());
  if (headers != dbheaders) {
    std::cout << "entity headers do not match\n";
    return -1;
  }
  initialized = 1;
  return 0;
}

int db::entities::populate_from_entry(str &entry, Entity *entity) {
  std::stringstream ss(entry);
  getline(ss, entity->profile_name, CSV_DELIMITER_c);
  getline(ss, entity->subject.cn, CSV_DELIMITER_c);
  getline(ss, entity->type, CSV_DELIMITER_c);
  getline(ss,entity->serial, CSV_DELIMITER_c);
  getline(ss, entity->subject.country, CSV_DELIMITER_c);
  getline(ss, entity->subject.state, CSV_DELIMITER_c);
  getline(ss, entity->subject.location, CSV_DELIMITER_c);
  getline(ss, entity->subject.organisation, CSV_DELIMITER_c);
  getline(ss, entity->subject.email, CSV_DELIMITER_c);
  getline(ss, entity->key_path, CSV_DELIMITER_c);
  getline(ss, entity->req_path, CSV_DELIMITER_c);
  getline(ss, entity->cert_path, CSV_DELIMITER_c);
  return 0;
}
int db::entities::populate_from_entry(str &entry,
                                      std::vector<std::string> &entity_fields) {
  str field;
  std::stringstream ss(entry);
  while (getline(ss, field, CSV_DELIMITER_c)) {
    entity_fields.push_back(field);
  }
  return 0;
}
int db::entities::populate_from_entry(str &profile, str &entry,str &cn,Entity &buff){
  std::ifstream file(DBDIR + profile + "_entities.csv");
  if(!file.is_open()){
    seterror(fmt::format("couldn't entities database for '{}'",profile));
    return -1;
  }
  sstream ss(entry);
  std::string _cn;
  getline(ss,_cn,CSV_DELIMITER_c);
  if(_cn == cn){
    return populate_from_entry(entry,&buff);
  }
  return 1;
}

int db::entities::exists(str &profile, strview common_name) {
  std::ifstream file(DBDIR +  profile + "_entities.csv");
  if (!file.is_open()) {
    return -1;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, &info);
    if (info.subject.cn == common_name) {
      return 1;
    }
  }
  return 0;
}

int db::entities::load(str &profile,
                       strview common_name, Entity *entity_buff) {
  std::ifstream file(DBDIR + profile + "_entities.csv");
  if (!file.is_open()) {
    return -1;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, &info);
    if (info.subject.cn == common_name) {
      *entity_buff = std::move(info);
      return 0;
    }
  }
  return -1;
}

int db::entities::add(Entity *entity) {
  Entity &e = *entity;
  str dbpath = _dbpath(entity->profile_name);
  std::ofstream file(dbpath, std::ios::app);
  if (!file.is_open()) {
    return -1;
  }
  uint64_t bsize = fs::file_size(dbpath);
  file << e.csv_entry() << std::endl;
  return !(fs::file_size(dbpath) > bsize);
}

int db::entities::del(str &profile, strview cn) {
  str dbpath = _dbpath(profile);
  if (exists(profile, cn)) {
    return -1;
  }
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return -1;
  }
  str line;
  Entity buff;
  fs::path tmpfilename = dbpath + ".tmp";
  std::ofstream tmpfile(tmpfilename);
  if (!tmpfile.is_open()) {
    return -1;
  }
  while (getline(file, line)) {
    populate_from_entry(line, &buff);
    if (buff.profile_name != profile) {
      tmpfile << line << std::endl;
    }
  }
  fs::remove(dbpath);
  fs::rename(tmpfilename, dbpath);
  return fs::exists(dbpath);
}