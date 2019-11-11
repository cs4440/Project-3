#include <cstring>
#include <iostream>
#include "../include/socket.h"

#define BUFLEN 1024

int main() {
    bool exit = false;
    char msg_size[4] = {0}, buf[BUFLEN] = {0};
    sock::Client client1("127.0.0.1");
    int sockfd, bytes, msg_len = 0;
    std::string line;

    try {
        client1.start();

        sockfd = client1.sockfd();

        while(!exit && std::getline(std::cin, line)) {
            // get line size and send to server the message length
            msg_len = line.size();
            memset(msg_size, 0, 4);
            sock::int_to_char(msg_len, msg_size);
            write(sockfd, msg_size, 4);

            // send to server the actual message
            write(sockfd, line.c_str(), line.size());

            if(strncmp(line.c_str(), "exit", 4) == 0) exit = true;
        }

    } catch(const std::exception &e) {
        std::cerr << "Client fail: " << e.what() << std::endl;
    }

    client1.stop();

    return 0;
}
