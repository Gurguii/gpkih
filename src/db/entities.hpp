#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "profiles.hpp"
#include "../structs.hpp"

class EntityManager{
private:
  std::unordered_map<std::string_view, gpkih::Entity> entities{};
  std::string dbpath;

public:
  static inline std::string db{};

  EntityManager(std::string_view profile_name);
  int sync();

  gpkih::Entity* const get(std::string_view cn);

  int del(std::string_view cn);

  int add(gpkih::Entity &entity);

  bool exists(std::string_view cn);
  bool exists(size_t serial);
  
  size_t size();

  const std::unordered_map<std::string_view, gpkih::Entity>* const retrieve();

  
};