#include <pthread.h>            // POSIX threads
#include <iostream>             // std::stream
#include "../include/disk.h"    // Disk class
#include "../include/parser.h"  // Parser, get cli tokens with grammar
#include "../include/socket.h"  // Socket class

#define BUFLEN 1024

void *connection_handler(void *socketfd);

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

void *connection_handler(void *socketfd) {
    bool exit = false;
    int sockfd = *(int *)socketfd;
    std::string client_msg;
    Parser parser;
    std::vector<Token> tokens;
    std::string diskfile = "client.disk";
    fs::Disk disk(diskfile);

    // static messages
    std::string welcome = "Welcome to server.\n";
    std::string need_create =
        "Please initialize disk with CREATE command: 'C [CYLINDERS] [SECTORS]'";
    std::string disk_exists =
        "ERROR disk already exists. Reusing existing disk";

    std::cout << "Serving client" << std::endl;

    // try to open disk if disk file exists
    try {
        if(disk.open_disk(diskfile))
            welcome += "Disk exists in system. Using existing disk: " +
                       std::to_string(disk.cylinder()) + " " +
                       std::to_string(disk.sector());
        else
            welcome += need_create;
    } catch(const std::exception &e) {
        welcome +=
            "ERROR. Initializating existing disk: " + std::string(e.what());
    }

    // Send welcome message to client
    sock::send_msg(sockfd, welcome);

    try {
        // read message from client
        // read first 4 bytes to determine message size
        while(!exit) {
            sock::read_msg(sockfd, client_msg);
            std::cout << client_msg << std::endl;

            if(client_msg.size()) {
                // tokenize/parse client message into arguments
                parser.clear();
                parser.set_string(client_msg.c_str());
                parser.parse();
                tokens = parser.get_tokens();

                // Exit
                if(tokens[0] == "exit") {
                    sock::send_msg(sockfd, "Closing client");
                    break;
                }
                // Create disk
                else if(tokens[0] == "C") {
                    int is_created = false;
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR. Insufficient arguments for C.");
                    else {
                        int cyl = std::stoi(tokens[1].string());
                        int sec = std::stoi(tokens[2].string());
                        disk.set_cylinders(cyl);
                        disk.set_sectors(sec);

                        try {
                            is_created = disk.create();

                            if(is_created)
                                sock::send_msg(sockfd, "Disk created.");
                            else {
                                sock::send_msg(sockfd, "ERROR. Disk exists.");
                            }
                        } catch(const std::exception &e) {
                            sock::send_msg(sockfd, e.what());
                        }
                    }
                }
                // Remove disk
                else if(tokens[0] == "D") {
                    int is_removed = false;
                    is_removed = disk.remove_disk();

                    if(is_removed)
                        sock::send_msg(sockfd, "1");
                    else
                        sock::send_msg(sockfd, "0");
                }
                // Get geometry information
                else if(tokens[0] == "I") {
                    if(disk.valid())
                        sock::send_msg(sockfd, disk.geometry());
                    else
                        sock::send_msg(sockfd,
                                       "ERROR. No disk.\n" + need_create);
                }
                // Read disk
                else if(tokens[0] == "R") {
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR. Insufficient arguments for R");
                    else {
                        if(disk.valid()) {
                            int cyl = std::stoi(tokens[1].string());
                            int sec = std::stoi(tokens[2].string());

                            std::string data = disk.read_at(cyl, sec);
                            sock::send_msg(sockfd, data);

                        } else
                            sock::send_msg(sockfd,
                                           "ERROR. No disk.\n" + need_create);
                    }
                }
                // Write disk
                else if(tokens[0] == "W") {
                    if(tokens.size() < 4)
                        sock::send_msg(sockfd,
                                       "ERROR. Insufficient arguments for W");
                    else {
                        if(disk.valid()) {
                            bool success = false;
                            int cyl = std::stoi(tokens[1].string());
                            int sec = std::stoi(tokens[2].string());

                            success =
                                disk.write_at(tokens[3].string().c_str(), cyl,
                                              sec, tokens[3].string().size());

                            if(success)
                                sock::send_msg(sockfd, "1");
                            else
                                sock::send_msg(sockfd, "0");
                        } else
                            sock::send_msg(sockfd,
                                           "ERROR. No disk.\n" + need_create);
                    }
                }
                // Unknown commands
                else {
                    sock::send_msg(sockfd, "Unknown command");
                }
            } else
                sock::send_msg(sockfd, "Unknown command");
        }
        std::cout << "Client called exit" << std::endl;
    } catch(const std::exception &e) {
        std::cout << "Client error. " << e.what() << std::endl;
    }

    delete(int *)socketfd;

    return 0;
}
