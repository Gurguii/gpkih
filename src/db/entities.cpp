#include "database.hpp"
#include <sstream>
using namespace gpkih;

/* Requires a call whenever we change the profile context */
int db::entities::initialize(str profile) {
  str dbpath = DB_DIRPATH + profile + "_entities.csv";
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
  getline(ss, entity.subject.cn, CSV_DELIMITER_c);
  getline(ss, buff, CSV_DELIMITER_c);
  entity.type = entity_type_map[buff];
  getline(ss, entity.serial, CSV_DELIMITER_c);
  getline(ss, entity.subject.country, CSV_DELIMITER_c);
  getline(ss, entity.subject.state, CSV_DELIMITER_c);
  getline(ss, entity.subject.location, CSV_DELIMITER_c);
  getline(ss, entity.subject.organisation, CSV_DELIMITER_c);
  getline(ss, entity.subject.email, CSV_DELIMITER_c);
  getline(ss, entity.key_path, CSV_DELIMITER_c);
  getline(ss, entity.csr_path, CSV_DELIMITER_c);
  getline(ss, entity.crt_path, CSV_DELIMITER_c);
  return GPKIH_OK;
}

int db::entities::populate_from_entry(str &entry,
                                      std::vector<std::string> &entity_fields) {
  str field;
  std::stringstream ss(entry);
  while (getline(ss, field, CSV_DELIMITER_c)) {
    entity_fields.push_back(field);
  }
  return GPKIH_OK;
}

int db::entities::populate_from_entry(str &profile, str &entry, str &cn,
                                      Entity &buff) {
  std::ifstream file(DB_DIRPATH + profile + "_entities.csv");
  if (!file.is_open()) {
    seterror(fmt::format("couldn't entities database for '{}'", profile));
    return GPKIH_FAIL;
  }
  sstream ss(entry);
  std::string _cn;
  getline(ss, _cn, CSV_DELIMITER_c);
  if (_cn == cn) {
    return populate_from_entry(entry, buff);
  }
  return 1;
}

int db::entities::exists(strview profile, strview common_name) {
  std::ifstream file(DB_DIRPATH + profile.data() + "_entities.csv");
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, info);
    if (info.subject.cn == common_name) {
      return 1;
    }
  }
  return GPKIH_FAIL;
}

int db::entities::load(str &profile, strview common_name, Entity &entity_buff) {
  std::ifstream file(DB_DIRPATH + profile + "_entities.csv");
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  str line;
  Entity info;
  while (getline(file, line)) {
    populate_from_entry(line, info);
    if (info.subject.cn == common_name) {
      entity_buff = std::move(info);
      return GPKIH_OK;
    }
  }
  return GPKIH_FAIL;
}

int db::entities::add(str &profile_name, Entity &entity) {
  std::cout << "adding entry: " << entity.csv_entry() << "\n";
  Entity &e = entity;
  str dbpath = std::move(_dbpath(profile_name));
  std::ofstream file(dbpath, std::ios::app);
  if (!file.is_open()) {
    return GPKIH_FAIL;
  }
  uint64_t bsize = fs::file_size(dbpath);
  file << e.csv_entry() << EOL;
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
