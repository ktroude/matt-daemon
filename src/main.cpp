#include "../include/Daemon.hpp"
#include <iostream>


int main() {
    try {
        Daemon daemon;
        return daemon.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
