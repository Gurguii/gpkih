#include "structs.hpp"
#include "../profiles/structs.hpp"
namespace gpkih::entity
{
  extern int setCAPaths(Profile &profile, Entity &entity);
  extern int setPaths(Profile &profile, Entity &entity);
  extern int loadSerial(Profile &profile, Entity &entity);
  extern int incrementSerial(Profile &profile, Entity &entity);

  // Experimental
  extern int setEntityFields(Profile &profile, Entity &entity);
}