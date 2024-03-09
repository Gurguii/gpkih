#pragma once

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/signal.h>
#endif

namespace gpkih::Signals
{
    extern void register_signals();
}; // namespace gpkih::Signals