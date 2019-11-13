#include <cstdlib>
#include <cstring>
#include <iostream>
#include "../include/socket.h"

int main(int argc, char* argv[]) {
    bool exit = false;
    sock::Client client;
    int port = 8000, sockfd;
    std::string host = "localhost", line, server_msg;

    if(argc > 1) host = argv[1];
    if(argc > 2) port = atoi(argv[2]);

    try {
        client.set_host(host);
        client.set_port(port);
        client.start();
        std::cout << "Client connected to server " << host << ":" << port
                  << std::endl;

        sockfd = client.sockfd();

        // read server welcome message
        sock::send_msg(sockfd, "welcome");
        sock::recv_msg(sockfd, server_msg);
        std::cout << server_msg << std::endl;

        while(!exit) {
            std::cout << "> ";
            std::getline(std::cin, line);

            if(line.size()) {
                sock::send_msg(sockfd, line);
                sock::recv_msg(sockfd, server_msg);
                std::cout << server_msg << std::endl;

                if(line == "exit") break;
            }
        }
    } catch(const std::exception& e) {
        std::cerr << "Server error on " << host << ":" << port << ". "
                  << e.what() << std::endl;
    }

    client.stop();

    return 0;
}
