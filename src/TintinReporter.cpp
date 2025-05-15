#include "../include/TintinReporter.hpp"
#include <sys/time.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <mutex>


/**
 * @brief Constructs a TintinReporter and opens the log file.
 * 
 * The log file is opened in append mode. If the file cannot be opened,
 * an exception is thrown.
 * 
 * @param logPath Path to the log file. Defaults to "../logs/matt_daemon.log" or a system path.
 * @throws std::runtime_error if the log file cannot be opened.
 */
TintinReporter::TintinReporter(const std::string &logPath) {
    logFile.open(logPath, std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Cannot open log file: " + logPath);
    }
}

/**
 * @brief Destructor for TintinReporter.
 * 
 * Closes the log file if it is open.
 */
TintinReporter::~TintinReporter() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief Logs a message with a timestamp and level.
 * 
 * Available levels: "INFO", "ERROR", any other is treated as "LOG".
 * Thread-safe via internal mutex.
 * 
 * @param level The type of log message ("INFO", "ERROR", "LOG", etc.)
 * @param message The content of the log message.
 */
void TintinReporter::log(const std::string &level, const std::string &message) {
    std::lock_guard<std::mutex> lock(logMutex);

    std::string log = getTimestamp();
    if (level == "INFO") {
        log += " [INFO] - Matt_daemon: " + message;
    } else if (level == "ERROR") {
        log += " [ERROR] - Matt_daemon: " + message;
    } else {
        log += " [LOG] - Matt_daemon: " + message;
    }

    logFile << log << std::endl;
}

/**
 * @brief Returns the current timestamp formatted as a string.
 * 
 * Format: [DD/MM/YYYY - HH:MM:SS]
 * 
 * @return Formatted timestamp string.
 */
std::string TintinReporter::getTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t t = tv.tv_sec;
    struct tm* tm = localtime(&t);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y - %H:%M:%S", tm);
    return std::string("[") + buffer + "]";
}
