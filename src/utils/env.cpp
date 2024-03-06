#include "utils.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <stdlib.h>
#endif

using namespace gpkih;

std::string utils::env::get_environment_variable(std::string_view varname)
{
    #ifdef _WIN32
    size_t size = 0;
    getenv_s(&size, NULL, 0, varname.data());
    if(size == 0){
        seterror("couldn't get env var '{}'", varname);
        return {};
    }
    std::string env("\0", size);
    getenv_s(&size, &env[0], env.size(), varname.data());

    return std::move(env);
    #else
    if(secure_getenv(varname.data()) == NULL){
        seterror("couldn't get env var '{}'", varname);
        return {};
    }
    return std::string{secure_getenv(varname.data())};
    #endif
};