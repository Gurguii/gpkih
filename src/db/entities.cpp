#include "database.hpp"
using namespace gpki;

int db::entities::initialize() {
  if (!std::filesystem::exists(dbpath)) {
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
  std::string headers(dbheaders.size(), '\x00');
  std::ifstream(dbpath).read(&headers[0], headers.size());
  if (headers != dbheaders) {
    std::cout << "entity headers do not match\n";
    return -1;
  }
  initialized = 1;
  return 0;
}
int db::entities::populate_from_entry(std::string &entry, Entity *entity) {
  std::string commas;
    std::stringstream ss(entry);
    getline(ss,entity->profile_name,',');
    getline(ss,entity->subject.cn,',');
    getline(ss,entity->type,',');
    getline(ss,entity->subject.country,',');
    getline(ss,entity->subject.state,',');
    getline(ss,entity->subject.location,',');
    getline(ss,entity->subject.organisation,',');
    getline(ss,entity->subject.email,',');
    getline(ss,entity->key_path,',');
    getline(ss,entity->req_path,',');
    getline(ss,entity->cert_path,',');
  return 0;
}
int db::entities::populate_from_entry(std::string &entry, std::vector<std::string> &fields){
  std::string field;
  std::stringstream ss(entry);
  while(getline(ss,field,CSV_DELIMITER_c)){
    fields.push_back(field);
  }
  return 0;
}

int db::entities::exists(std::string_view profile, std::string_view common_name){
  std::ifstream file(dbpath);
  if(!file.is_open()){
    return -1;
  }
  std::string line;
  Entity info;
  while(getline(file,line)){
    populate_from_entry(line,&info);
    if(info.subject.cn == common_name){
      return 1;
    }
  }
  return 0;
}
int db::entities::load(Profile *profile, Entity *entity_buff, std::string_view common_name){
  std::ifstream file(dbpath);
  if(!file.is_open()){
    return -1;
  }
  std::string line;
  Entity info;
  while(getline(file,line)){
    populate_from_entry(line,&info);
    if(info.subject.cn == common_name){
      *entity_buff = std::move(info);
      return 0;
    }
  }
  return -1;
}
int db::entities::add(Entity *entity) {
  Entity &e = *entity; 
  std::ofstream file(dbpath,std::ios::app);
  if(!file.is_open()){
    return -1;
  }
  uint64_t bsize = std::filesystem::file_size(dbpath);
  file << e.csv_entry() << std::endl;
  return !(std::filesystem::file_size(dbpath) > bsize);
}

int db::entities::del(std::string_view profile, std::string_view cn) { 
  if(exists(profile,cn)){
    return -1;
  }
  std::ifstream file(dbpath);
  if(!file.is_open()){
    return -1;
  }
  std::string line;
  Entity buff;

  std::filesystem::path tmpfilename = dbpath + ".tmp";
  std::ofstream tmpfile(tmpfilename);
  if(!tmpfile.is_open()){
    return -1;
  }
  while(getline(file,line)){
    populate_from_entry(line,&buff);
    if(buff.profile_name != profile){
      tmpfile <<  line << std::endl;
    }
  }
  std::filesystem::remove(dbpath);
  std::filesystem::rename(tmpfilename,dbpath);
  return std::filesystem::exists(dbpath);
}
