#include "utils.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace gpkih;

std::string utils::env::get_environment_variable(std::string_view varname)
{
    #ifdef _WIN32
    // [Windows]
    size_t size = 0;
    getenv_s(&size, nullptr, 0, varname.data());
    if(size == 0){
        return {};
    }
    std::string env("\0", size);
    getenv_s(&size, &env[0], env.size(), varname.data());

    return std::move(env);
    #else
    // [Linux]
    if(secure_getenv(varname.data()) == nullptr){
        return {};
    }
    return std::string{secure_getenv(varname.data())};
    #endif
};