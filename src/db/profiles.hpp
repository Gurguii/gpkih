#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "../structs.hpp"
#include "../utils/utils.hpp"
#include "../memory/memmgmt.hpp"
#include "mnck.hpp"

namespace gpkih::db::profiles 
{
  /* 
	  - Profiles will be stored in binary form
	  - Each line will contain a profile

    syntax: <id><namelen><name><sourcelen><source><creation_date><last_modification><ca><total_servers><total_clients>
    	
    namelen: uint8_t indicating length of name
    name: char array
	
		sourcelen: uint8_t indicating source length
		source: char array
	 
		ca: uint8_t indicating ca presence - 1:yes 0:no
	
		total_servers: size_t containing amount of server certificates created
		total_clients: size_t containing amount of client certificates created
  	
  	note: adding, removing or modying existing_profiles won't
  	take effect until `db::profiles::sync()` is called.
  */
  
  inline std::string dbpath{}; // set by main()
  inline std::string idfile{}; // set by main()                         

  extern std::map <std::string_view, Profile>* const get();

  /// @brief writes existing_profiles to db, effectively updating values if any got modified after the last initialize()
  /// @return GPKIH_OK if profiles in existing_profiles where succesfully overriden in profiles.data, else GPKIH_FAIL
  extern int sync();  		

  /// @param loaded_profiles ptr to size_t that will hold number of profiles loaded
  /// @return number of loaded profiles or GPKIH_FAIL (-1)
  extern int initialize(size_t &loaded_profiles);					    
  
  /// @param profile_name profile to check
  /// @return true/false based on profile existance  
  extern bool exists(std::string_view profile_name);
    
  /// @param profile struct Profile to add to profiles.data
  /// @return GPKIH_OK on success else GPKIH_FAIL
  extern int add(Profile &profile);									              
      
  /// @param profile_name name of profile to remove
  /// @param GPKIH_OK on success else GPKIH_FAIL
  extern int remove(std::string_view profile_name);					      
  
  /// @return number of profiles deleted or -1 if any error
  extern int remove_all();											                  
  
  /// @param profile_id id of desired profile
  /// @param pinfo sturct Profile buffer to load data to  
  extern int load(uint64_t profile_id, Profile &pinfo);
  
  /// @param profile_name name of the profile to load
  /// @param pinfo struct Profile buffer to load data to
  extern int load(std::string_view profile_name, Profile &pinfo); 
  
  /// @param profile_id id of desired profile
  extern Profile *const get(uint64_t profile_id);

  /// @param profile_name name of desired profile
  extern Profile *const get(std::string_view profile_name);

  /// @return existing_profiles size
  extern size_t size();

} // namespace gpkih::db::profiles