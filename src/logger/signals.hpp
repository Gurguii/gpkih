#pragma once
#include "../gpki.hpp"
#include "logger.hpp"

namespace gpkih
{
class Signals {
private:
#ifdef _WIN32
    static BOOL ctrl_c_handler(DWORD signal);
#else
    static void ctrl_c_handler(int sig);
#endif
    
public:
    static void register_signals();
};
} // namespace gpkih