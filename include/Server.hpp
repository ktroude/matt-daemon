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
        Server(int port, TintinReporter &logger);
        ~Server();
    
        void start();
        void stop();
        void handleClient(int clientSocket);
    
    private:
        int serverSocket;
        struct sockaddr_in serverAddr;
        TintinReporter &logger;
        std::vector<std::thread> clientThreads;
        std::mutex clientMutex;
        std::atomic<bool> running;
        std::atomic<int> activeClients;
    };
    

#endif
