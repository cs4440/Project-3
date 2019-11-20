#include <pthread.h>            // POSIX threads
#include <iostream>             // std::stream
#include "../include/disk.h"    // Disk class
#include "../include/parser.h"  // Parser, get cli tokens with grammar
#include "../include/socket.h"  // Socket class

// GLOBALS
int TRACK_TIME = 10;  // in microseconds
int CYLINDERS = 5;    // default cylinders
int SECTORS = 10;     // default sectors per cylinders

void *connection_handler(void *socketfd);

int main(int argc, char *argv[]) {
    int port = 8000;
    sock::Server server;
    int newsockfd, *thsockfd = nullptr;
    pthread_t tid;
    pthread_attr_t attr;

    if(argc > 1) port = atoi(argv[1]);
    if(argc > 2) TRACK_TIME = atoi(argv[2]);
    if(argc > 3) CYLINDERS = atoi(argv[3]);
    if(argc > 4) SECTORS = atoi(argv[4]);

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
    // get sockfd and deallocate argument
    int sockfd = *(int *)socketfd;
    delete(int *)socketfd;
    socketfd = nullptr;

    bool exit = false;
    std::string client_msg;
    Parser parser;
    std::vector<std::string> tokens;
    std::string diskname = "client";

    // create disk with default settings
    fs::Disk disk(diskname, CYLINDERS, SECTORS);
    disk.set_track_time(TRACK_TIME);

    // static messages
    std::string welcome =
        "WELCOME TO SERVER\n\n"
        "Available commands are:\n"
        "[C]reate - Create/initialize disk. 'C [CYL] [SEC]'\n"
        "[D]elete - Delete current disk\n"
        "[I]nfo - Get disk geometry information\n"
        "[R]ead - Read from disk. 'R [CYL] [SEC]'\n"
        "[W]rite - Write to disk. 'W [CYL] [SEC] [DATA]'\n\n";
    std::string need_create =
        "Please initialize disk with CREATE command: 'C [CYL] [SEC]'";
    std::string disk_exists =
        "ERROR disk already exists. Reusing existing disk";

    std::cout << "Serving client" << std::endl;

    // try to open disk if disk file exists
    try {
        if(disk.open(diskname))
            welcome += "Disk exists in system. Using existing disk: " +
                       std::to_string(disk.cylinder()) + " " +
                       std::to_string(disk.sector());
        else
            welcome += need_create;
    } catch(const std::exception &e) {
        welcome +=
            "ERROR Initializating existing disk: " + std::string(e.what());
    }

    try {
        while(!exit) {
            sock::recv_msg(sockfd, client_msg);
            std::cout << client_msg << std::endl;

            if(client_msg.size()) {
                // tokenize/parse client message into arguments
                parser.clear();
                parser.set_string(client_msg.c_str());
                parser.parse();
                tokens = parser.get_tokens();

                // Exit
                if(tokens[0] == "exit") {
                    std::cout << "Client requested exit" << std::endl;
                    sock::send_msg(sockfd, "Closing client");
                    exit = true;
                }
                // Send welcome message to client
                else if(tokens[0] == "welcome")
                    sock::send_msg(sockfd, welcome);
                // Send ping response with 1
                else if(tokens[0] == "ping")
                    sock::send_msg(sockfd, "1");
                // Create disk
                else if(tokens[0] == "C") {
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for C.");
                    else {
                        try {
                            if(!disk.valid()) {
                                int cyl = std::stoi(tokens[1]);
                                int sec = std::stoi(tokens[2]);

                                disk.set_cylinders(cyl);
                                disk.set_sectors(sec);
                                disk.create();

                                sock::send_msg(
                                    sockfd, std::to_string(disk.cylinder()) +
                                                " " +
                                                std::to_string(disk.sector()));
                            } else {
                                sock::send_msg(sockfd, "ERROR Disk exists.");
                            }
                        } catch(const std::exception &e) {
                            sock::send_msg(sockfd, e.what());
                        }
                    }
                }
                // Remove disk
                else if(tokens[0] == "D") {
                    if(disk.remove())
                        sock::send_msg(sockfd, "1");
                    else
                        sock::send_msg(sockfd, "0");
                }
                // Get geometry information
                else if(tokens[0] == "I") {
                    if(disk.valid())
                        sock::send_msg(sockfd, disk.geometry());
                    else
                        sock::send_msg(sockfd, "0 0\n" + need_create);
                }
                // Read disk
                else if(tokens[0] == "R") {
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for R");
                    else {
                        if(disk.valid()) {
                            int cyl = std::stoi(tokens[1]);
                            int sec = std::stoi(tokens[2]);

                            std::string data = disk.read_at(cyl, sec);
                            sock::send_msg(sockfd, data);

                        } else
                            sock::send_msg(sockfd,
                                           "ERROR No disk.\n" + need_create);
                    }
                }
                // Write disk
                else if(tokens[0] == "W") {
                    if(tokens.size() < 4)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for W");
                    else {
                        if(disk.valid()) {
                            bool success = false;
                            int cyl = std::stoi(tokens[1]);
                            int sec = std::stoi(tokens[2]);

                            success = disk.write_at(tokens[3].c_str(), cyl, sec,
                                                    tokens[3].size());

                            if(success)
                                sock::send_msg(sockfd, "1");
                            else
                                sock::send_msg(sockfd, "0");
                        } else
                            sock::send_msg(sockfd,
                                           "ERROR No disk.\n" + need_create);
                    }
                }
                // Unknown commands
                else
                    sock::send_msg(sockfd, "Unknown command");
            } else
                sock::send_msg(sockfd, "Unknown command");
        }
    } catch(const std::exception &e) {
        std::cout << "Client error. " << e.what() << std::endl;
    }

    close(sockfd);

    return 0;
}
