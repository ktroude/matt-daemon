#include "../include/Server.hpp"
#include <string.h>
#include <unistd.h>
#include <algorithm>

Server::Server(int port, TintinReporter &logger): logger(logger), running(true), activeClients(0)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        logger.log("ERROR", "Server: socket creation failed.");
        throw std::runtime_error("Server: socket creation failed");
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serverAddr.sin_zero), 0, 8);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        logger.log("ERROR", "Server: bind failed.");
        throw std::runtime_error("Server: bind failed");
    }

    if (listen(serverSocket, 3) < 0) {
        logger.log("ERROR", "Server: listen failed, maximum socket reached.");
        throw std::runtime_error("Server: listen failed, maximum socket reached");
    }

    logger.log("INFO", "Server created.");
}

Server::~Server() {
    if (serverSocket >= 0) {
        close(serverSocket);
    }

    std::lock_guard<std::mutex> lock(clientMutex);
    for (std::thread& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    logger.log("INFO", "Server destroyed.");
}

void Server::start() {
    while (running) {
        socklen_t addrLen = sizeof(sockaddr_in);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &addrLen);

        if (clientSocket >= 0) {
            if (activeClients < 3) {
                std::lock_guard<std::mutex> lock(clientMutex);
                clientThreads.emplace_back(&Server::handleClient, this, clientSocket); // 👈 utiliser this ici
                logger.log("INFO", "Client " + std::to_string(clientSocket) + " connected");
                activeClients++;
            } else {
                logger.log("ERROR", "Maximum socket reached");
                close(clientSocket);
            }
        }
    }
}

void Server::stop() {
    running = false;
    shutdown(serverSocket, SHUT_RDWR);
}

void Server::handleClient(int clientSocket) {
    char buffer[1024];

    while (this->running) {
        ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (received == 0) {
            logger.log("INFO", "Client disconnected.");
            break;
        }
        if (received < 0) {
            logger.log("ERROR", "recv error.");
            break;
        }

        buffer[received] = '\0';

        std::string input(buffer);
        input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
        input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());

        if (input == "quit") {
            logger.log("INFO", "Request quit.");
            this->stop();
        } else {
            logger.log("LOG", "User input: " + input);
        }
    }

    close(clientSocket);
    this->activeClients--;
    logger.log("INFO", "Client handler thread exited.");
}