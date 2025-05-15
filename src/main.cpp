#include "../include/Daemon.hpp"

int main() {
    try {
        Daemon daemon;
        return daemon.run();
    } catch (const std::exception& e) {
        return 1;
    }
}
