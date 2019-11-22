#include <pthread.h>            // POSIX threads
#include <iostream>             // std::stream
#include "../include/disk.h"    // Disk class
#include "../include/fat.h"     // Disk class
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
    std::string diskname = "client-fs-basic";

    // create disk with default settings
    fs::FatFS fatfs;
    fs::Disk disk(diskname, CYLINDERS, SECTORS);
    disk.set_track_time(TRACK_TIME);

    // static messages
    std::string welcome =
        "WELCOME TO BASIC FILESYSTEM SERVER\n\n"
        "FILE SYSTEM COMMANDS:\n"
        "---------------------\n"
        "[F]ormat filesystem of disk size [CYLINDER] [SECTOR]: 'F [C] [S]'\n"
        "[C]reate file: 'C [NAME]'\n"
        "[D]elete file: 'D [NAME]'\n"
        "[L]ist current dirs/files, flag=0 minimal, flag=1 full: 'L [FLAG]'\n"
        "[R]ead a file: 'R [NAME]'\n"
        "[W]rite data to file: 'W [NAME] [DATA]'\n"
        "[I]nformation of file system: name, valid, size (in bytes), etc\n"
        "[U]nformat a filesystem and deletes disk\n\n";
    std::string need_create = "Please format filesystem with 'F' command";
    std::string disk_exists = "ERROR filesystem exists";

    std::cout << "Serving client" << std::endl;

    // try to open disk if disk file exists
    try {
        if(disk.open(diskname)) {
            fatfs.set_disk(&disk);
            fatfs.open_disk();
            welcome +=
                "Filessytem exists in server. Using existing file system\n" +
                fatfs.info();
        } else
            welcome += need_create;
    } catch(const std::exception &e) {
        welcome += "ERROR Initializating existing disk/filesystem: " +
                   std::string(e.what());
    }

    try {
        while(!exit) {
            sock::recv_msg(sockfd, client_msg);
            std::cout << client_msg << std::endl;

            if(client_msg.size()) {
                try {
                    // tokenize/parse client message into arguments
                    parser.clear();
                    parser.set_string(client_msg.c_str());
                    parser.parse();
                    tokens = parser.get_tokens();
                } catch(const std::exception &e) {
                    sock::send_msg(sockfd, "1 ERROR Command too long");
                    continue;
                }

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
                else if(tokens[0] == "C") {
                    if(tokens.size() < 2)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for C");
                    else {
                        try {
                            fatfs.add_file(tokens[1]);
                            sock::send_msg(sockfd, "0 Created");

                        } catch(const std::invalid_argument &e) {
                            sock::send_msg(sockfd,
                                           "1 " + std::string(e.what()));
                        } catch(const std::exception &e) {
                            sock::send_msg(sockfd,
                                           "2 " + std::string(e.what()));
                        }
                    }
                } else if(tokens[0] == "D") {
                    if(tokens.size() < 2)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for D");
                    else {
                        bool is_deleted = fatfs.delete_file(tokens[1]);

                        if(is_deleted)
                            sock::send_msg(sockfd, "0 Deleted");
                        else
                            sock::send_msg(sockfd, "1 No file exist");
                    }
                }
                // Create disk
                else if(tokens[0] == "F") {
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for F");
                    else if(fatfs.valid())
                        sock::send_msg(sockfd, "ERROR Filesystem exists.");
                    else {
                        int cylinders = std::stoi(tokens[1]);
                        int sectors = std::stoi(tokens[2]);

                        disk.set_cylinders(cylinders);
                        disk.set_sectors(sectors);
                        disk.create();

                        fatfs.set_disk(&disk);
                        fatfs.format();

                        sock::send_msg(sockfd, fatfs.info());
                    }
                } else if(tokens[0] == "I") {
                    sock::send_msg(sockfd, fatfs.info());
                } else if(tokens[0] == "L") {
                    std::ostringstream oss;
                    fatfs.print_dirs(oss);
                    fatfs.print_files(oss);

                    sock::send_msg(sockfd, oss.str());
                } else if(tokens[0] == "R") {
                    if(tokens.size() < 2)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for R");
                    else {
                        try {
                            fs::FileEntry file = fatfs.find_file(tokens[1]);

                            if(!file)
                                sock::send_msg(sockfd, "1 No file exists");
                            else {
                                int bytes = 0;
                                char *data = new char[file.data_size() + 1];
                                bytes = fatfs.read_file_data(file, data,
                                                             file.size());
                                data[bytes] = '\0';

                                sock::send_msg(
                                    sockfd,
                                    "0 " + std::to_string(bytes) + " " + data);

                                delete[] data;
                            }
                        } catch(const std::exception &e) {
                            sock::send_msg(sockfd,
                                           "2 " + std::string(e.what()));
                        }
                    }
                } else if(tokens[0] == "W") {
                    if(tokens.size() < 3)
                        sock::send_msg(sockfd,
                                       "ERROR Insufficient arguments for W");
                    else {
                        try {
                            fs::FileEntry file = fatfs.find_file(tokens[1]);

                            if(!file)
                                sock::send_msg(sockfd, "1 No file exists");
                            else {
                                fatfs.write_file_data(file, tokens[2].c_str(),
                                                      tokens[2].size());

                                sock::send_msg(sockfd, "0");
                            }
                        } catch(const std::exception &e) {
                            sock::send_msg(sockfd,
                                           "2 " + std::string(e.what()));
                        }
                    }
                } else if(tokens[0] == "U") {
                    fatfs.remove();
                    sock::send_msg(sockfd, "File system and disk removed");
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
