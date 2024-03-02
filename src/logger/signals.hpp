#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/signal.h>
#endif

namespace gpkih::Signals
{
#ifdef _WIN32
    extern BOOL ctrl_c_handler(DWORD signal);
#else
    extern void ctrl_c_handler(int sig);
#endif
    extern void register_signals();
}; // namespace gpkih::Signals