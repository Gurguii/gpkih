#pragma once

#include "../entities/structs.hpp"
#include <string>
#include <map>

class EntityManager{
private:
  static inline std::string dbDir{}; 
  std::map<std::string_view, Entity> entities{};
  std::string dbpath;
public:
  /// @brief absolute path aiming to logs root dir [LINUX]$HOME [WIN]$env:home 
  static const std::string &setDir(std::string_view path);
  static const std::string &getDir();
  
  /// @param profile_name name of the profile to load Entities from
  /// @brief construct a new EntityManager instance
  EntityManager(std::string_view profile_name);

  /// @brief synchronizes file data with Entities within private member `entities`
  /// @return GPKIH_OK on success, else GPKIH_FAIL
  int sync();

  /// @brief get constant pointer to Entity within entities' umap
  /// @param cn common_name of Entity to get
  /// @return valid Entity* or nullptr if not found
  Entity* const get(std::string_view cn);

  /// @brief deletes entity from umap 
  /// @param cn common name of target entity
  /// @return GPKIH_OK on succesful deletion, else GPKIH_FAIL
  int del(std::string_view cn);

  /// @brief adds entity to the umap
  /// @paran entity struct Entity to add 
  /// @return GPKIH_OK on succesfull insertion, else GPKIH_FAIL
  int add(Entity &entity);

  /// @brief checks entity existance based on common_name
  /// @param cn common_name of target entity
  /// @return true if exists, else false
  bool exists(std::string_view cn);

  /// @brief checks entity existance based on serial
  /// @param serial serial of target entity
  /// @return true if exists, else false 
  bool exists(size_t serial);
  
  /// @brief checks entity existance based on common name
  /// @param cn common_name of target entity
  /// @param buff struct Entity that will point to the Entity if found
  /// @return true if Entity exists, else false
  bool exists(std::string_view cn, Entity* &buff);

  /// @return entities umap's size
  size_t size();

  /// @return true|false indicating if map is empty
  bool empty(); 

  /// @brief get the private umap
  /// @return constant pointer to umap<std::string_view,Entity>
  const std::map<std::string_view, Entity>* const retrieve();
};