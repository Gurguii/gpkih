#include "../conv.hpp"
#include <unordered_map>

using namespace gpkih::entity;

std::string conversion::toString(ENTITY_TYPE type)
{
  switch(type){
  	case ET_CA:
  	  return "ca";
  	case ET_CL:
  	  return "cl";
  	case ET_SV:
  	  return "sv";
  	default:
  	  return "none";
  }
};

std::string conversion::toString(ENTITY_STATUS status)
{
  switch(status){
    case ES_ACTIVE:
      return "active";
    case ES_REVOKED:
      return "revoked";
    case ES_MARKED:
      return "marked";
    default:
      return "unknown";
  }
}

template <>
ENTITY_TYPE conversion::toEnum<ENTITY_TYPE>(std::string_view enumName){
  {
    static std::unordered_map<std::string_view, ENTITY_TYPE> _map{
      {"cl",ET_CL},
      {"client",ET_CL},
      {"sv",ET_SV},
      {"server", ET_SV},
      {"ca", ET_CA}
    };
    if(_map.find(enumName) == _map.end()){
      return {};
    }
    return _map[enumName];
  };
}
template <>
ENTITY_STATUS conversion::toEnum<ENTITY_STATUS>(std::string_view enumName){
  {
    static std::unordered_map<std::string_view, ENTITY_STATUS> _map{
      {"active",ES_ACTIVE},
      {"revoked",ES_REVOKED},
      {"marked",ES_MARKED}
    };
    if(_map.find(enumName) == _map.end()){
      return {};
    }
    return _map[enumName];
  };
}