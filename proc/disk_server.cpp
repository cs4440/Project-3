#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "../include/socket.h"

#define BUFLEN 1024

void *connection_handler(void *socketfd) {
    bool exit = false;
    int sockfd = *(int *)socketfd;
    char msg_len[4] = {0}, buf[BUFLEN] = {0};
    int msg_size = 0, bytes = 0, totalbytes = 0;

    std::cout << "Serving client" << std::endl;

    // read first 4 bytes to determine messge size
    while(!exit && (bytes = read(sockfd, msg_len, 4)) > 0) {
        sock::char_to_int(msg_len, msg_size);  // convert 4 byte char to int

        // keep reading from socket until msg_sze is reached
        while(!exit && totalbytes < msg_size) {
            bytes = read(sockfd, buf, BUFLEN - 1);
            buf[bytes] = '\0';

            std::cout << buf << std::endl;

            if(strncmp(buf, "exit", 4) == 0) exit = true;

            totalbytes += bytes;
        }
        bytes = 0;
        totalbytes = 0;
    }

    std::cout << "Client called exit" << std::endl;

    delete(int *)socketfd;

    return 0;
}

int main(int argc, char *argv[]) {
    int port = 8000;
    sock::Server server;
    int newsockfd, *thsockfd = nullptr;
    pthread_t tid;
    pthread_attr_t attr;

    if(argc > 1) port = atoi(argv[1]);

    try {
        server.set_port(port);
        server.start();
        std::cout << "Server started on port " << port << std::endl;

        // set pthread attributes to detach
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        // listen to incoming connections
        while((newsockfd = server.accept_connection()) > -1) {
            thsockfd = new int;
            *thsockfd = newsockfd;

            if(newsockfd < 0)
                std::cerr << "ERROR on socket accept" << std::endl;
            else {
                if(pthread_create(&tid, NULL, connection_handler,
                                  (void *)thsockfd)) {
                    std::cerr << "ERROR on creating new thread" << std::endl;
                }
            }
        }
    } catch(const std::exception &e) {
        std::cerr << "Server fail: " << e.what() << std::endl;
    }

    server.stop();

    return 0;
}
