#include "../entities.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

int entity::setCAPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 15; // 15 = /pki/ca/key.pem | /pki/ca/crt.pem
  e.crtPathLen = e.keyPathLen;
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == nullptr || e.crtPath == nullptr){
    PERROR("smth is null\n");
    return GPKIH_FAIL;
  }

  if(e.keyPathLen != snprintf(e.keyPath,e.keyPathLen+1,"%s%cpki%cca%ckey.pem",profile.source,SLASH,SLASH,SLASH)
    ||
     e.crtPathLen != snprintf(e.crtPath,e.crtPathLen+1,"%s%cpki%cca%ccrt.pem",profile.source,SLASH,SLASH,SLASH)){
    PERROR("snprintf() returned different than len\nkey:{}:%lu crt:{}:%lu\n",e.keyPath, e.keyPathLen, e.crtPath, e.crtPathLen);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

int entity::setPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/keys/-key.pem 
  e.crtPathLen = profile.sourcelen + 19 + e.subject.cnlen; // /pki/certs/-crt.pem
  e.csrPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/reqs/-csr.pem
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.csrPath = ALLOCATE(e.csrPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == nullptr || e.csrPath == nullptr || e.crtPath == nullptr){
    return GPKIH_FAIL;
  }

  if(e.keyPathLen != snprintf(e.keyPath, e.keyPathLen+1,"%s%cpki%ckeys%c%s-key.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.csrPathLen != snprintf(e.csrPath, e.csrPathLen+1, "%s%cpki%creqs%c%s-csr.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.crtPathLen != snprintf(e.crtPath, e.crtPathLen+1, "%s%cpki%ccerts%c%s-crt.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)){
    PERROR("snprintf() returned different than len\nkey:{} csr:{} crt:{}\n",e.keyPath, e.csrPath, e.crtPath);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

int entity::incrementSerial(Profile &profile, Entity &entity){
  PDEBUG(2,"__increment_serial()");
  std::string serialPath = fmt::format("{}{}pki{}serial{}serial", profile.source, SLASH, SLASH, SLASH);

  std::ifstream rfile(serialPath);
  
  if(!rfile.is_open()){
    PERROR("Couldn't open serial file '{}' to update serial\n", serialPath);
    return GPKIH_FAIL;
  }

  std::string nserial{};
  rfile >> nserial;

  nserial = fmt::format("{:x}",static_cast<decltype(Entity::serial)>(std::stoull(nserial,0,16) + 1));

  rfile.close();
  
  std::ofstream wfile(serialPath);
  if(!wfile.is_open()){
    return GPKIH_FAIL;
  }

  if(nserial.size() == 1){
    wfile << "0";
  }

  wfile << nserial;
  
  wfile.close();

  return GPKIH_OK;
}

int entity::loadSerial(Profile &profile, Entity &entity){
  PDEBUG(2,"__load_serial()");
  std::string serialPath = fmt::format("{}{}pki{}serial{}serial", profile.source, SLASH, SLASH, SLASH);

  if(!std::filesystem::exists(serialPath)){
    PERROR("serial file for profile '{}' not found - '{}'\n", profile.name, serialPath);
    return GPKIH_FAIL;
  }

  std::ifstream file(serialPath);

  if(!file.is_open()){
    PERROR("couldn't open file '{}'\n",serialPath);
    return GPKIH_FAIL;
  }

  std::string serial{};
  file >> serial;
  file.close();

  try{
    entity.serial = std::stoull(serial,nullptr,16);  
  }catch(const std::invalid_argument &err){
    PINFO("Loading serial failed, generating random serial [{}]...\n", err.what());
    entity.serial = rand();
  }

  return GPKIH_OK;
}