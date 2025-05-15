#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "TintinReporter.hpp"
#include "Server.hpp"

class Daemon {
public:
    Daemon();
    ~Daemon();

    int run();

private:
    void daemonize();
    bool createLockFile();
    void removeLockFile();
    void setupSignalHandlers();
    static void handleSignal(int signal);

    static Daemon* instance;
    TintinReporter logger;
    Server* server;
    int lockFd;
    bool running;
};

#endif
