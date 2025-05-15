#include "../include/Server.hpp"
#include <string.h>
#include <unistd.h>
#include <algorithm>

/**
 * @brief Constructs the server, creates a TCP socket bound to the given port and starts listening.
 * 
 * @param port The port on which the server will listen.
 * @param logger Reference to the TintinReporter for logging server events.
 * @throws std::runtime_error If socket creation, bind or listen fails.
 */
Server::Server(int port, TintinReporter &logger) : logger(logger), activeClients(0), running(true) 
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Server: socket creation failed");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serverAddr.sin_zero), 0, 8);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Server: bind failed");
    }

    if (listen(serverSocket, 3) < 0) {
        throw std::runtime_error("Server: listen failed, maximum socket reached");
    }

    this->logger.log("INFO", "Server created.");
}

/**
 * @brief Destructor for the Server.
 * Closes the server socket and joins all client threads gracefully.
 */
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

/**
 * @brief Starts the server loop and waits for client connections.
 * 
 * Accepts up to 3 clients simultaneously and creates a thread for each.
 * Clients exceeding the limit are rejected.
 */
void Server::start()
{
    while (running) {
        socklen_t addrLen = sizeof(sockaddr_in);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &addrLen);

        if (clientSocket >= 0) {
            if (activeClients < 3) {
                std::lock_guard<std::mutex> lock(clientMutex);
                clientThreads.emplace_back(&Server::handleClient, clientSocket, std::ref(logger), this);
                activeClients++;                
            } else {
                logger.log("ERROR", "Maximum socket reached");
                close(clientSocket);
            }
        }

    }
}

/**
 * @brief Stops the server by disabling the accept loop and shutting down the socket.
 * 
 * This allows accept() to unblock and the server to exit cleanly.
 */
void Server::stop()
{
    running = false;
    shutdown(serverSocket, SHUT_RDWR);
}

/**
 * @brief Handles communication with a connected client.
 * 
 * Reads data from the client. If "quit" is received, the server is stopped.
 * All other messages are logged.
 * 
 * @param clientSocket The socket descriptor of the connected client.
 * @param logger Reference to the TintinReporter for logging client activity.
 * @param srv Pointer to the main Server instance, used to call stop() and access shared state.
 */
void Server::handleClient(int clientSocket, TintinReporter &logger, Server *srv)
{
    char buffer[1024];

    while (srv->running) {
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
            srv->stop();
            break;
        } else {
            logger.log("LOG", "User input: " + input);
        }
    }

    close(clientSocket);
    --srv->activeClients;
    logger.log("INFO", "Client handler thread exited.");
}
