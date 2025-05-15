#include "../include/Daemon.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

/**
 * @brief Constructs the daemon and initializes its members.
 * 
 * Initializes the logger with the log path, sets the default state,
 * and registers this instance as the global signal handler context.
 */
Daemon::Daemon() : logger("/var/log/matt_daemon/matt_daemon.log"), server(nullptr), lockFd(-1), running(true) {
    instance = this;
}

/**
 * @brief Destructor for the daemon.
 * 
 * Performs no action since cleanup is handled in run() and signal handler.
 */
Daemon::~Daemon() {}

/**
 * @brief Entry point for the daemon.
 * 
 * Checks for root privileges, creates a lock file to ensure singleton instance,
 * daemonizes the process, creates the log directory, starts the server,
 * sets up signal handling, and waits for client communication.
 * 
 * @return int Exit code: 0 on success, 1 on error.
 * @throws std::runtime_error If daemon cannot initialize properly.
 */
int Daemon::run()
{
    uid_t id = geteuid();

    if (id != 0) {
        throw std::runtime_error("Daemon: root privilege needed");
    }

    bool isCreated = createLockFile();

    if (!isCreated) {
        throw std::runtime_error("Daemon: lock file error");
    }

    daemonize();

    mkdir("/var/log/matt_daemon/", 0700);
    server = new Server(4242, logger);

    setupSignalHandlers();
    logger.log("INFO", "Entering Daemon mode.");
    server->start();

    removeLockFile();
    delete server;
    return 0;
}

/**
 * @brief Converts the process into a Unix daemon.
 * 
 * Forks the process, detaches it from the terminal, starts a new session,
 * changes working directory to root, sets file mode creation mask,
 * and redirects standard I/O to /dev/null.
 * 
 * @throws std::runtime_error If fork or setsid fails.
 */
void Daemon::daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        throw std::runtime_error("Daemon: fork error");
    }
    if (pid > 0) {
        exit(0);
    }

    if (setsid() < 0) {
        throw std::runtime_error("Daemon: setsid error");
    }

    chdir("/");
    umask(0);

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > 2) {
        close(fd);
    } 

}

/**
 * @brief Creates a lock file to ensure only one daemon instance is running.
 * 
 * Attempts to create /var/lock/matt_daemon.lock using O_EXCL. 
 * If the file already exists, it means another daemon is running.
 * 
 * @return true If the lock file was successfully created.
 * @return false If the file already exists or cannot be created.
 */
bool Daemon::createLockFile() {
    lockFd = open("/var/lock/matt_daemon.lock", O_CREAT | O_EXCL | O_WRONLY, 0644);

    if (lockFd < 0) {
        logger.log("ERROR", "Can not create lock file.");
        return false;
    }
    return true;
}

/**
 * @brief Removes the lock file and closes the file descriptor.
 * 
 * This function is called during shutdown and in the signal handler
 * to ensure proper cleanup.
 */
void Daemon::removeLockFile() {
    if (lockFd >= 0) {
        close(lockFd);
    }
    
    unlink("/var/lock/matt_daemon.lock");

}

/**
 * @brief Sets up signal handlers for clean shutdown.
 * 
 * Registers custom handlers for SIGINT and SIGTERM using sigaction.
 * These handlers stop the server and remove the lock file gracefully.
 */
void Daemon::setupSignalHandlers() {
    instance = this;
    struct sigaction signal;
    signal.sa_handler = Daemon::handleSignal;
    sigemptyset(&signal.sa_mask);
    signal.sa_flags = 0;
    sigaction(SIGINT, &signal, nullptr);
    sigaction(SIGTERM, &signal, nullptr);
}

/**
 * @brief Static signal handler callback.
 * 
 * Called when a registered signal is received.
 * Stops the server, logs the event, removes the lock file,
 * and deallocates the server instance.
 * 
 * @param signal The received signal number.
 */
void Daemon::handleSignal(int signal) {
    if (instance) {
        instance->logger.log("INFO", "Signal received: " + std::to_string(signal));
        instance->server->stop();
        instance->removeLockFile();
        delete instance->server;
        instance->server = nullptr;
    }
}
