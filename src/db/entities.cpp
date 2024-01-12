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
    db << dbheaders;
    db.close();
    std::cout << "entity csv created\n";
    return 0;
  }
  std::string headers(dbheaders.size(), '\x00');
  std::ifstream(dbpath).read(&headers[0], headers.size());
  if (headers != dbheaders) {
    std::cout << "entity headers do not match\n";
  }
  std::cout << "[!] entities' csv created\n";
  return 0;
}
int db::entities::populate_entry(std::string entry, Entity *entity) {
  std::ifstream file(dbpath);
  if (!file.is_open()) {
    return -1;
  }
  std::string line;
  std::string commas;
  while (getline(file, line)) {
    std::stringstream ss(line);
    ss >> entity->profile_name;
    ss >> commas;
    ss >> entity->subject.cn;
    ss >> commas;
    ss >> entity->subject.country;
    ss >> commas;
    ss >> entity->subject.state;
    ss >> commas;
    ss >> entity->subject.location;
    ss >> commas;
    ss >> entity->subject.organisation;
    ss >> commas;
    ss >> entity->subject.email;
    ss >> commas;
    ss >> entity->key_path;
    ss >> commas;
    ss >> entity->req_path;
    ss >> commas;
    ss >> entity->cert_path;
  }
  return 0;
}
int db::entities::exists(Entity *entity) { return 0; }
int db::entities::add(Entity *entity) { return 0; }
int db::entities::del(Entity *entity) { return 0; }
