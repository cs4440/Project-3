#include <cstdlib>
#include <cstring>
#include <iostream>
#include "../include/socket.h"

#define BUFLEN 1024

int main(int argc, char* argv[]) {
    bool exit = false;
    char msg_len[4] = {0}, buf[BUFLEN] = {0};
    sock::Client client;
    int port = 8000, sockfd, bytes, msg_size = 0;
    std::string host = "localhost", line;

    if(argc > 1) host = argv[1];
    if(argc > 2) port = atoi(argv[2]);

    try {
        client.set_host(host);
        client.set_port(port);
        client.start();
        std::cout << "Client started on host " << host << ":" << port << std::endl;

        sockfd = client.sockfd();

        while(!exit) {
            std::cout << "> ";
            std::getline(std::cin, line);

            // get line size and send to server the message length
            msg_size = line.size();
            sock::int_to_char(msg_size, msg_len);  // convert to 4 byte char
            write(sockfd, msg_len, 4);

            // send to server the actual message
            write(sockfd, line.c_str(), line.size());

            if(strncmp(line.c_str(), "exit", 4) == 0) exit = true;
        }

    } catch(const std::exception& e) {
        std::cerr << "Client fail on host " << host << ":" << port << ", " << e.what() << std::endl;;
    }

    client.stop();

    return 0;
}
