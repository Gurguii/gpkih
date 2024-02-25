#include "signals.hpp"

using namespace gpkih;
#ifdef _WIN32
    BOOL Signals::ctrl_c_handler(DWORD signal){
        return TRUE;
    }
#else
    void Signals::ctrl_c_handler(int sig){
        Logger::cleanup_with_exit();
    };
#endif

void Signals::register_signals(){
    #ifdef _WIN32
        SetConsoleCtrlHandler(ctrl_c_handler, true);
    #else
        signal(SIGINT, ctrl_c_handler);
    #endif
}
