#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "TintinReporter.hpp"

class Server {

    public:
        Server(int port, TintinReporter& logger);
        ~Server();

        void start();
        void stop();

    private:
        int serverSocket;
        struct sockaddr_in serverAddr;
        std::vector<std::thread> clientThreads;
        std::mutex clientMutex;
        std::atomic<int> activeClients;
        bool running;

        TintinReporter& logger;

        static void handleClient(int clientSocket, TintinReporter& logger, Server* srv);
};

#endif
