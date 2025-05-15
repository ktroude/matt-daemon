#ifndef TINTIN_REPORTER_HPP
#define TINTIN_REPORTER_HPP

#include <string>
#include <fstream>
#include <mutex>

class TintinReporter {

    public:
        TintinReporter(const std::string& logPath);
        ~TintinReporter();

        void log(const std::string& level, const std::string& message);

    private:
        std::ofstream logFile;
        std::mutex logMutex;

        std::string getTimestamp();

};

#endif
