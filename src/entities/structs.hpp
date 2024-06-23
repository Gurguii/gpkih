#pragma once
#include "enums.hpp"
#include <chrono>
/**
 * @struct Subject
 * @brief Information about the subject of an Entity
 * @var Subject::country 3 letter code indicating country
 * @var Subject::state state
 * @var Subject::statelen state len
 * @var Subject::location location
 * @var Subject::locationlen location len
 * @var Subject::organisation organisation
 * @var Subject::organisationlen organisation len
 * @var Subject::cn common name (unique per profile)
 * @var Subject::cnlen common name len
 * @var Subject::email email 
 * @var Subject::emaillen email len
 * */
struct Subject {
  char country[3]{0};

  char *state = nullptr;
  uint8_t statelen = 0;

  char *location = nullptr;
  uint8_t locationlen = 0; 

  char *organisation = nullptr;
  uint8_t organisationlen = 0;
  
  char *cn = nullptr;
  uint8_t cnlen = 0;
  
  char *email = nullptr;
  uint8_t emaillen = 0;

  Subject () = default;

};

/**
 * @struct keyInfo
 * @brief holds metadata about Entity's key
 * @var keyInfo::algo key algorithm, ed25519, rsa ...
 * @var keyInfo::algoLen len of keyInfo::algo 
 * @var keyInfo::keySize size of key (bits)
* */ 
struct keyInfo{
  char *algo = nullptr; // ed25519, rsa...
  uint8_t algoLen;
  size_t size;
};

// Idk about this, you can retrieve days by substracting expirationDate and creationDate...
// the x509 extensions are hardcoded for clients | servers | ca's (no need to store it)
// what else?? 
struct crtInfo{
  int days = 0;
};

/**
 * @struct Entity
 * @brief represents the entity behind each created certificate, holds information about the entity itself and the subject
 * @var Entity::subject see \ref Subject
 * @var Entity::type custom enum type indicating the type of entity, valid types are CA,SV,CL, values also include NONE
 * @var Entity::status custom enum type indicating entity status, valid types are ACTIVE,REVOKED,MARKED(not implemented a way to mark yet)
 * @var Entity::serial unique serial
 * @var Entity::creationDate time when Entity was created @note not a string type
 * @var Entity::keyPath absolute path of file containing private key
 * @var Entity::keyPathLen size of keyPath
 * @var Entity::csrPath absolute path of file containing the certificate signing request
 * @var Entity::csrPathLen size of csrPath
 * @var Entity::crtPath absolute path of file containing the signed certificate
 * @var Entity::crtPathLen size of crtPath
 * */
struct Entity{
  size_t serial;
  
  Subject subject;

  ENTITY_TYPE type = ET_NONE;
  ENTITY_STATUS status = ES_NONE;

  std::chrono::time_point<std::chrono::system_clock> creationDate = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> expirationDate; // set by build action based on days configuration

  char *keyPath = nullptr;
  uint32_t keyPathLen = 0;

  char *csrPath = nullptr;
  uint32_t csrPathLen = 0;

  char *crtPath = nullptr;
  uint32_t crtPathLen = 0;

  // !NEW-UNIMPLEMENTED - these members aren't loaded into/from data files
  keyInfo *keyMetadata = nullptr;
  crtInfo *crtMetadata = nullptr;

  Entity() = default;
};