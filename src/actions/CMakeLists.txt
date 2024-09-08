cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0079 NEW) 
project(gpkih VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 
add_subdirectory("export")

# Add subdirectories for individual actions
file(GLOB_RECURSE add_sources LIST_DIRECTORIES true "add/*.cpp")
file(GLOB_RECURSE gencrl_sources LIST_DIRECTORIES true "gencrl/*.cpp")
file(GLOB_RECURSE get_sources LIST_DIRECTORIES true "get/*.cpp")
file(GLOB_RECURSE init_sources LIST_DIRECTORIES true "init/*.cpp")
file(GLOB_RECURSE list_sources LIST_DIRECTORIES true "list/*.cpp")
file(GLOB_RECURSE remove_sources LIST_DIRECTORIES true "remove/*.cpp")
file(GLOB_RECURSE rename_sources LIST_DIRECTORIES true "rename/*.cpp")
file(GLOB_RECURSE reset_sources LIST_DIRECTORIES true "reset/*.cpp")
file(GLOB_RECURSE revoke_sources LIST_DIRECTORIES true "revoke/*.cpp")
file(GLOB_RECURSE set_sources LIST_DIRECTORIES true "set/*.cpp")

set(sources
	"actions.cpp"
    ${add_sources} 
    ${gencrl_sources} 
    ${get_sources} 
    ${init_sources} 
    ${list_sources} 
    ${remove_sources} 
    ${rename_sources} 
    ${reset_sources} 
    ${revoke_sources} 
    ${set_sources}
)

# Create a library for the actions
add_library(gpkih_actions_lib STATIC ${sources})
target_link_libraries(gpkih_actions_lib PRIVATE gpkih_actions_export)
target_include_directories(gpkih_actions_lib PUBLIC "${FMT_LIBRARY_INCLUDE}")