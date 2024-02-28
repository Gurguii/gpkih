#include "database.hpp"
#include <sstream> // std::stringstream
#include <fstream> // std::ifstream | std::ofstream

using namespace gpkih;

/* Requires a call whenever we change the profile context */
int db::entities::initialize(str profile) {
  str dbpath = gpkih::db::profiles::dbpath + profile + "_entities.csv";
  if (!fs::exists(dbpath)) {
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      seterror("couldn't create file '{}'",dbpath);
      return GPKIH_FAIL;
    }
    db << dbheaders << std::endl;
    db.close();
    return GPKIH_OK;
  }
  str headers(dbheaders.size(), '\x00');
  std::ifstream(dbpath).read(&headers[0], headers.size());
  if (headers != dbheaders) {
    seterror("entity headers do not match\n");
    return GPKIH_FAIL;
  }
  return GPKIH_OK;
}

int db::entities::populate_from_entry(str &entry, Entity &entity) {
  std::stringstream ss(entry);
  str buff;
  getline(ss, entity.subject.cn, ',');
  getline(ss, buff, ',');
  entity.type = entity_type_map[buff];
  getline(ss, entity.serial, ',');
  getline(ss, entity.subject.country, ',');
  getline(ss, entity.subject.state, ',');
  getline(ss, entity.subject.location, ',');
  getline(ss, entity.subject.organisation, ',');
  getline(ss, entity.subject.email, ',');
  getline(ss, entity.key_path, ',');
  getline(ss, entity.csr_path, ',');
  getline(ss, entity.crt_path, ',');
  return GPKIH_OK;  
}

int db::entities::populate_from_entry(str &entry,
                                      std::vector<std::string> &entity_fields) {
  str field;
  std::stringstream ss(entry);
  while (getline(ss, field, ',')) {
    entity_fields.push_back(field);
  }
  return GPKIH_OK;
}

// what the fk is this
int db::entities::populate_from_entry(str &profile, str &entry, str &cn,
                                      Entity &buff) {
  std::ifstream file(gpkih::db::profiles::dbpath + profile + "_entities.csv");
  if (!file.is_open()) {
    seterror(fmt::format("couldn't entities database for '{}'", profile));
    return GPKIH_FAIL;
  }
  sstream ss(entry);
  std::string _cn;
  getline(ss, _cn, ',');
  file.close();
  if (_cn == cn) {
    return populate_from_entry(entry, buff);
  }
  return 1;
}

int db::entities::exists(strview profile, strview common_name) {
  std::ifstream file(gpkih::db::profiles::dbpath + profile.data() + "_entities.csv");
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, info);
    if (info.subject.cn == common_name) {
      file.close();
      return ENTITY_FOUND;
    }
  }
  file.close();
  return GPKIH_FAIL;
}

int db::entities::load(str &profile, strview common_name, Entity &entity_buff) {
  std::ifstream file(gpkih::db::profiles::dbpath + profile + "_entities.csv");
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, info);
    if (info.subject.cn == common_name) {
      entity_buff = std::move(info);
      file.close();
      return GPKIH_OK;
    }
  }
  file.close();
  return GPKIH_FAIL;
}

int db::entities::add(str &profile_name, Entity &entity) {
  Entity &e = entity;
  str dbpath = std::move(_dbpath(profile_name));
  std::ofstream file(dbpath, std::ios::app);
  if (!file.is_open()) {
    file.close();
    return GPKIH_FAIL;
  }
  uint64_t bsize = fs::file_size(dbpath);
  file << e.csv_entry() << EOL;
  file.close();
  return GPKIH_OK;
}

int db::entities::del(str &profile, strview cn) {
  str dbpath = _dbpath(profile);
  if (exists(profile, cn)) {
    return GPKIH_FAIL;
  }
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  str line;
  Entity buff;
  fs::path tmpfilename = dbpath + ".tmp";
  std::ofstream tmpfile(tmpfilename);
  if (!tmpfile.is_open()) {
    return GPKIH_FAIL;
  }
  while (getline(file, line)) {
    populate_from_entry(line, buff);
    if (buff.subject.cn != cn) {
      tmpfile << line << std::endl;
    }
  }
  tmpfile.close();
  fs::remove(dbpath);
  fs::rename(tmpfilename, dbpath);
  return fs::exists(dbpath);
}
