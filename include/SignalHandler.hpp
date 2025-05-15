#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP

#include <functional>
#include "TintinReporter.hpp"

class SignalHandler {
public:
    static void setup(TintinReporter& logger, std::function<void()> cleanupCallback);
};

#endif
