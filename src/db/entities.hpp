#pragma once

#include <string>
#include <string_view>
#include "../structs.hpp"

class EntityManager{
private:
  std::unordered_map<std::string_view, gpkih::Entity> entities{};
  std::string dbpath;
  size_t current_size = 0;
public:
  // @brief absolute path aiming to logs root dir [LINUX]$HOME [WIN]$env:home 
  static inline std::string dbdir{};

  // @param profile_name name of the profile to load Entities from
  // @brief construct a new EntityManager instance
  EntityManager(std::string_view profile_name);

  // @brief synchronizes file data with Entities within private member `entities`
  // @return GPKIH_OK on success, else GPKIH_FAIL
  int sync();

  // @brief get constant pointer to Entity within entities' umap
  // @param cn common_name of Entity to get
  // @return valid Entity* or nullptr if not found
  gpkih::Entity* const get(std::string_view cn);

  // @brief deletes entity from umap 
  // @param cn common name of target entity
  // @return GPKIH_OK on succesful deletion, else GPKIH_FAIL
  int del(std::string_view cn);

  // @brief adds entity to the umap
  // @paran entity struct Entity to add 
  // @return GPKIH_OK on succesfull insertion, else GPKIH_FAIL
  int add(gpkih::Entity &entity);

  // @brief checks entity existance based on common_name
  // @param cn common_name of target entity
  // @return true if exists, else false
  bool exists(std::string_view cn);

  // @brief checks entity existance based on serial
  // @param serial serial of target entity
  // @return true if exists, else false 
  bool exists(size_t serial);
  
  // @brief checks entity existance based on common name
  // @param cn common_name of target entity
  // @param buff struct Entity that will point to the Entity if found
  // @return true if Entity exists, else false
  bool exists(std::string_view cn, gpkih::Entity* &buff);

  // @return entities umap's size
  size_t size();

  // @brief get the private umap
  // @return constant pointer to umap<strview,Entity>
  const std::unordered_map<std::string_view, gpkih::Entity>* const retrieve();
  
};