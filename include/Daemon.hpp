#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "TintinReporter.hpp"
#include "Server.hpp"

class Daemon {
public:
    Daemon();
    ~Daemon();

    int run();
    void removeLockFile();

    static Daemon* instance;

private:
    void daemonize();
    bool createLockFile();
    void setupSignalHandlers();
    static void handleSignal(int signal);

    TintinReporter logger;
    Server* server;
    int lockFd;
    bool running;
};

#endif
