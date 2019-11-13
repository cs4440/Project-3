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
        std::cout << "Client started on host " << host << ":" << port
                  << std::endl;

        sockfd = client.sockfd();

        // read server welcome message
        sock::read_msg(sockfd, server_msg);
        std::cout << server_msg << std::endl;

        while(!exit) {
            std::cout << "> ";
            std::getline(std::cin, line);

            if(line.size()) {
                sock::send_msg(sockfd, line);
                sock::read_msg(sockfd, server_msg);
                std::cout << server_msg << std::endl;

                if(line == "exit") break;
            }
        }

    } catch(const std::exception& e) {
        std::cerr << "Client error on host " << host << ":" << port << ". "
                  << e.what() << std::endl;
        ;
    }

    client.stop();

    return 0;
}
