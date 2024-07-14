#include "../entities.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

int entity::setCAPaths(Profile &profile, Entity &e){
  e.meta.keyPathLen = profile.meta.sourceLen + 15; // 15 = /pki/ca/key.pem | /pki/ca/crt.pem
  e.meta.crtPathLen = e.meta.keyPathLen;
  
  e.keyPath = reinterpret_cast<const char *>(ALLOCATE(e.meta.keyPathLen + 1));
  e.crtPath = reinterpret_cast<const char *>(ALLOCATE(e.meta.crtPathLen + 1));

  if(e.keyPath == nullptr || e.crtPath == nullptr){
    PERROR("smth is null\n");
    return GPKIH_FAIL;
  }

  if(e.meta.keyPathLen != snprintf(const_cast<char*>(e.keyPath),e.meta.keyPathLen+1,"%s%cpki%cca%ckey.pem",profile.source,SLASH,SLASH,SLASH)
    ||
     e.meta.crtPathLen != snprintf(const_cast<char*>(e.crtPath),e.meta.crtPathLen+1,"%s%cpki%cca%ccrt.pem",profile.source,SLASH,SLASH,SLASH)){
    PERROR("snprintf() returned different than len\nkey:{}:%lu crt:{}:%lu\n",e.keyPath, e.meta.keyPathLen, e.crtPath, e.meta.crtPathLen);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

int entity::setPaths(Profile &profile, Entity &e){
  DEBUG(1, "entity::setPaths()");

  e.meta.keyPathLen = profile.meta.sourceLen + 18 + e.subject.meta.cnlen; // /pki/keys/-key.pem 
  e.meta.crtPathLen = profile.meta.sourceLen + 19 + e.subject.meta.cnlen; // /pki/certs/-crt.pem
  e.meta.csrPathLen = profile.meta.sourceLen + 18 + e.subject.meta.cnlen; // /pki/reqs/-csr.pem
  
  e.keyPath = reinterpret_cast<const char*>(ALLOCATE(e.meta.keyPathLen + 1));
  e.csrPath = reinterpret_cast<const char*>(ALLOCATE(e.meta.csrPathLen + 1));
  e.crtPath = reinterpret_cast<const char*>(ALLOCATE(e.meta.crtPathLen + 1));

  if(e.keyPath == nullptr || e.csrPath == nullptr || e.crtPath == nullptr){
    return GPKIH_FAIL;
  }

  if(e.meta.keyPathLen != snprintf(const_cast<char*>(e.keyPath), e.meta.keyPathLen+1,"%s%cpki%ckeys%c%s-key.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.meta.csrPathLen != snprintf(const_cast<char*>(e.csrPath), e.meta.csrPathLen+1, "%s%cpki%creqs%c%s-csr.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.meta.crtPathLen != snprintf(const_cast<char*>(e.crtPath), e.meta.crtPathLen+1, "%s%cpki%ccerts%c%s-crt.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)){
    DEBUGF(2, "snprintf() returned different than len\nkey:{} csr:{} crt:{}\n",e.keyPath, e.csrPath, e.crtPath);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

int entity::incrementSerial(Profile &profile, Entity &entity){
  DEBUG(1,"__increment_serial()");
  std::string serialPath = fmt::format("{}{}pki{}serial{}serial", profile.source, SLASH, SLASH, SLASH);

  std::ifstream rfile(serialPath);
  
  if(!rfile.is_open()){
    PERROR("Couldn't open serial file '{}'\n", serialPath);
    return GPKIH_FAIL;
  }

  std::string nserial{};
  rfile >> nserial;

  nserial = fmt::format("{:x}",static_cast<decltype(EntityMetadata::serial)>(std::stoull(nserial,0,16) + 1));

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
  DEBUG(2,"__load_serial()");
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
    entity.meta.serial = std::stoull(serial,nullptr,16);  
  }catch(const std::invalid_argument &err){
    PINFO("Loading serial failed, generating random serial [error:{}]...\n", err.what());
    entity.meta.serial = rand();
  }

  return GPKIH_OK;
}

void entity::setExpirationDate(Entity &entity, size_t days){
  entity.meta.expirationDate = entity.meta.creationDate + std::chrono::seconds(3600*24*days);
}