#include "database.hpp"
using namespace gpki;

int db::entities::initialize() {
  // entry e.g
  if (!std::filesystem::exists(dbpath)) {
    std::ofstream db(dbpath);
    if (!db.is_open()) {
      std::cout << "couldn't create file '" << dbpath << "'\n";
      return -1;
    }
    db << dbheaders << std::endl;
    db.close();
    std::cout << "entity csv created\n";
    return 0;
  }
  std::string headers(dbheaders.size(), '\x00');
  std::ifstream(dbpath).read(&headers[0], headers.size());
  if (headers != dbheaders) {
    std::cout << "entity headers do not match\n";
  }
  std::cout << "[!] entities' csv OK\n";
  return 0;
}
int db::entities::populate_entry(std::string &entry, Entity *entity) {
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
int db::entities::exists(Entity *entity) { 
  std::ifstream file(dbpath);
  if(!file.is_open()){
    return -1;
  }
  std::string line;
  Entity entry;
  while(getline(file,line)){
    populate_entry(line,&entry);
    if(entry.subject.cn == entity->subject.cn && entry.profile_name == entity->profile_name){
      // entity exists
      return 1;
    }
  }
  return 0;
}

int db::entities::add(Entity *entity) {
  Entity &e = *entity; 
  std::ofstream file(dbpath,std::ios::app);
  if(!file.is_open()){
    return -1;
  }
  uint64_t bsize = std::filesystem::file_size(dbpath);
  file << e.profile_name << "," << e.subject.cn << "," << e.type << "," << e.subject.country << "," << e.subject.state << "," << e.subject.location << "," << e.subject.organisation << "," << e.subject.email << "," << e.key_path << "," << e.req_path << "," << e.cert_path << std::endl;
  uint64_t asize = std::filesystem::file_size(dbpath);
  return !(asize > bsize);
}

int db::entities::del(Entity *entity) { 
  Entity &e = *entity;
  if(exists(entity)){
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
    populate_entry(line,&buff);
    if(buff.profile_name != e.profile_name){
      tmpfile <<  line << std::endl;
    }
  }
  std::filesystem::remove(dbpath);
  std::filesystem::rename(tmpfilename,dbpath);
  return std::filesystem::exists(dbpath);
}