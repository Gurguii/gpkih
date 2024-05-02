#include "signals.hpp"


using namespace gpkih;

#ifdef _WIN32
    static BOOL ctrl_c_handler(DWORD signal) {
        ExitProcess(0);
        return TRUE;
    }
#else
    static void ctrl_c_handler(int sig){
        exit(0);
        //Logger::cleanup_with_exit();
    };
#endif

void Signals::register_signals(){
    #ifdef _WIN32
        SetConsoleCtrlHandler(ctrl_c_handler, true);
    #else
        signal(SIGINT, ctrl_c_handler);
    #endif
}
