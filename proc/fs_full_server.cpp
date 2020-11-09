#include <pthread.h>  // POSIX threads
#include <unistd.h>   // getopt()

#include <iostream>  // std::stream
#include <sstream>   // ostringstream

#include "../include/disk.h"    // Disk class
#include "../include/fat.h"     // Disk class
#include "../include/parser.h"  // Parser, get cli tokens with grammar
#include "../include/socket.h"  // socket Server class

// GLOBALS
int TRACK_TIME = 10;  // in microseconds
int CYLINDERS = 5;    // default cylinders
int SECTORS = 10;     // default sectors per cylinders

// Structure for connection handler argument
struct connection_info {
    int sockfd;
    struct sockaddr_in client_addr;
};

// thread function when a connection is accepted
void *connection_handler(void *con_info);

// FUNCTIONS TO HANDLE SERVER COMMANDS
namespace fs {

// make a file system
void mkfs(int sockfd, std::vector<std::string> &tokens, fs::Disk &disk,
          fs::FatFS &fatfs);

// remove file system
void rmfs(int sockfd, fs::FatFS &fatfs);

// make a directory
void mkdir(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// remove a directory
void rmdir(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// make a file
void mk(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// remove a file
void rm(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// read data from file
void read(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// write data to file
void write(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// append data to file
void append(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// change to path
void cd(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// list path contents
void ls(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs);

// print working directory
void pwd(int sockfd, fs::FatFS &fatfs);

}  // namespace fs

int main(int argc, char *argv[]) {
    int port = 8000;
    sock::Server server;
    int newsockfd = -1;
    pthread_t tid;
    pthread_attr_t attr;
    connection_info *con_info;

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
            con_info = new connection_info;
            con_info->sockfd = newsockfd;
            con_info->client_addr = server.client_addr();

            if(newsockfd < 0)
                std::cerr << "ERROR on socket accept" << std::endl;
            else {
                if(pthread_create(&tid, NULL, connection_handler,
                                  (void *)con_info)) {
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

void *connection_handler(void *con_info) {
    // get sockfd and client_addr
    struct connection_info *con = (struct connection_info *)con_info;
    int sockfd = con->sockfd;
    struct sockaddr_in client_addr = con->client_addr;

    // get client address IPv4
    struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&client_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    // convert IPv4 to string
    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, ipv4, INET_ADDRSTRLEN);

    // get port
    socklen_t len = sizeof(client_addr);
    getpeername(sockfd, (struct sockaddr *)&client_addr, &len);
    unsigned short int port = client_addr.sin_port;

    // deallocate argument
    delete(connection_info *)con_info;
    con_info = nullptr;

    // fs variables
    bool exit = false;
    std::string client_msg;
    Parser parser;
    std::vector<std::string> tokens;
    std::string diskname = "client-fs-full";

    // create disk with default settings
    fs::FatFS fatfs;
    fs::Disk disk(diskname, CYLINDERS, SECTORS);
    disk.set_track_time(TRACK_TIME);

    // static messages
    std::string unknown_cmd = "Command not found";
    std::string welcome =
        "WELCOME TO FILESYSTEM SERVER\n\n"
        "FILE SYSTEM COMMANDS:\n"
        "---------------------\n"
        "mkfs [CYLINDER] [SECTOR]\tCreate filesystem size of cylinder x "
        "sector\n"
        "rmfs\t\t\t\tRemove filesystem\n"
        "mkdir [NAME]\t\t\tCreate a directory entry\n"
        "rmdir [NAME]\t\t\tRemove a directory\n"
        "mk [NAME]\t\t\tCreate a file entry\n"
        "rm [NAME]\t\t\tRemove a file\n"
        "read [NAME]\t\t\tRead file data\n"
        "write [NAME] [DATA]\t\tWrite data to file\n"
        "append [NAME] [DATA]\t\tAppend data to file\n"
        "cd [PATH]\t\t\tChange directory to PATH\n"
        "ls\t\t\t\tList path contents\n"
        "pwd\t\t\t\tList path contents\n"
        "info\t\t\t\tDisplay current filesystem information\n\n";
    std::string need_create =
        "Please create and format filesystem with 'mkfs' command";
    std::string disk_exists = "ERROR filesystem exists";

    std::cout << "Serving client@" << ipv4 << ":" << port << std::endl;

    // try to open disk if disk file exists
    try {
        if(disk.open(diskname)) {
            fatfs.set_disk(&disk);
            fatfs.open_disk();
            welcome +=
                "Filessytem exists in server. Using existing file system\n";
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
                    std::cout << "error" << std::endl;
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
                else if(tokens[0] == "mkfs" || tokens[0] == "F")
                    fs::mkfs(sockfd, tokens, disk, fatfs);
                else if(tokens[0] == "rmfs" || tokens[0] == "U")
                    fs::rmfs(sockfd, fatfs);
                else if(tokens[0] == "mkdir")
                    fs::mkdir(sockfd, tokens, fatfs);
                else if(tokens[0] == "rmdir")
                    fs::rmdir(sockfd, tokens, fatfs);
                else if(tokens[0] == "mk" || tokens[0] == "C")
                    fs::mk(sockfd, tokens, fatfs);
                else if(tokens[0] == "rm" || tokens[0] == "D")
                    fs::rm(sockfd, tokens, fatfs);
                else if(tokens[0] == "read" || tokens[0] == "R")
                    fs::read(sockfd, tokens, fatfs);
                else if(tokens[0] == "write" || tokens[0] == "W")
                    fs::write(sockfd, tokens, fatfs);
                else if(tokens[0] == "append" || tokens[0] == "A")
                    fs::append(sockfd, tokens, fatfs);
                else if(tokens[0] == "cd")
                    fs::cd(sockfd, tokens, fatfs);
                else if(tokens[0] == "ls" || tokens[0] == "L")
                    fs::ls(sockfd, tokens, fatfs);
                else if(tokens[0] == "pwd")
                    fs::pwd(sockfd, fatfs);
                else if(tokens[0] == "info" || tokens[0] == "I")
                    sock::send_msg(sockfd, fatfs.info());
                // Unknown commands
                else
                    sock::send_msg(sockfd, unknown_cmd);
            } else
                sock::send_msg(sockfd, unknown_cmd);
        }
    } catch(const std::exception &e) {
        std::cout << "Client error. " << e.what() << std::endl;
    }

    close(sockfd);

    return 0;
}

namespace fs {

void mkfs(int sockfd, std::vector<std::string> &tokens, fs::Disk &disk,
          fs::FatFS &fatfs) {
    if(tokens.size() < 3)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for mkfs");
    else if(fatfs.valid())
        sock::send_msg(sockfd, "ERROR Filesystem exists.");
    else {
        try {
            int cylinders = std::stoi(tokens[1]);
            int sectors = std::stoi(tokens[2]);

            disk.set_cylinders(cylinders);
            disk.set_sectors(sectors);
            disk.create();

            fatfs.set_disk(&disk);
            fatfs.format();

            sock::send_msg(sockfd, fatfs.info());
        } catch(const std::exception &e) {
            disk.remove();
            fatfs.remove();
            sock::send_msg(sockfd, "1 " + std::string(e.what()));
        }
    }
}

void rmfs(int sockfd, fs::FatFS &fatfs) {
    fatfs.remove();
    sock::send_msg(sockfd, "File system and disk removed");
}

void mkdir(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for mkdir");
    else {
        try {
            fatfs.add_dir(tokens[1]);
            sock::send_msg(sockfd, "0 Created");

        } catch(const std::invalid_argument &e) {
            sock::send_msg(sockfd, "1 " + std::string(e.what()));
        } catch(const std::exception &e) {
            sock::send_msg(sockfd, "2 " + std::string(e.what()));
        }
    }
}

void rmdir(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for rmdir");
    else {
        if(fatfs.delete_dir(tokens[1]))
            sock::send_msg(sockfd, "0 Deleted");
        else
            sock::send_msg(sockfd, "1 No such file or directory");
    }
}

void mk(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for mkfile");
    else {
        try {
            fatfs.add_file(tokens[1]);
            sock::send_msg(sockfd, "0 Created");

        } catch(const std::invalid_argument &e) {
            sock::send_msg(sockfd, "1 " + std::string(e.what()));
        } catch(const std::exception &e) {
            sock::send_msg(sockfd, "2 " + std::string(e.what()));
        }
    }
}

void rm(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for rm");
    else {
        if(fatfs.delete_file(tokens[1]))
            sock::send_msg(sockfd, "0 Deleted");
        else
            sock::send_msg(sockfd, "1 No such file or directory");
    }
}

void read(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for read");
    else {
        try {
            fs::FileEntry file = fatfs.find_file(tokens[1]);

            if(!file)
                sock::send_msg(sockfd, "1 No file exists");
            else {
                int bytes = 0;
                char *data = new char[file.data_size() + 1];
                bytes = fatfs.read_file_data(file, data, file.size());
                data[bytes] = '\0';

                sock::send_msg(sockfd,
                               "0 " + std::to_string(bytes) + " " + data);

                delete[] data;
            }
        } catch(const std::exception &e) {
            sock::send_msg(sockfd, "2 " + std::string(e.what()));
        }
    }
}

void write(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 3)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for write");
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
            sock::send_msg(sockfd, "2 " + std::string(e.what()));
        }
    }
}

void append(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    if(tokens.size() < 3)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for write");
    else {
        try {
            fs::FileEntry file = fatfs.find_file(tokens[1]);

            if(!file)
                sock::send_msg(sockfd, "1 No file exists");
            else {
                fatfs.append_file_data(file, tokens[2].c_str(),
                                       tokens[2].size());

                sock::send_msg(sockfd, "0");
            }
        } catch(const std::exception &e) {
            sock::send_msg(sockfd, "2 " + std::string(e.what()));
        }
    }
}

void cd(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    // TODO PARSE PATH AND CHANGE TO FULL PATH
    if(tokens.size() < 2)
        sock::send_msg(sockfd, "ERROR Insufficient arguments for cd");
    else {
        if(fatfs.valid()) {
            if(fatfs.change_dir(tokens[1]))
                sock::send_msg(sockfd, "");
            else
                sock::send_msg(sockfd, "1 No such directory");
        } else
            sock::send_msg(sockfd, "1 No filesystem");
    }
}

void ls(int sockfd, std::vector<std::string> &tokens, fs::FatFS &fatfs) {
    bool is_details = false;
    char opts[] = "1l";
    char **argv = nullptr;
    int argc = tokens.size(), opt = -1;
    std::ostringstream oss;
    std::string ls_path = ".";
    std::string *path = &ls_path;

    // converts tokens into argv
    argv = new char *[tokens.size() + 1];
    for(std::size_t i = 0; i < tokens.size(); ++i) {
        argv[i] = new char[tokens[i].size() + 1]();
        strncpy(argv[i], tokens[i].c_str(), tokens[i].size());
    }
    argv[tokens.size()] = nullptr;

    // get hyphen options
    optind = 0;  // reset global option index from <unistd> extern
    while((opt = getopt(argc, argv, opts)) != -1) {
        switch(opt) {
            case '1':
            case 'l':
                is_details = true;
                break;
            default:
                break;
        }
    }

    // deallocate argv
    for(std::size_t i = 0; i < tokens.size(); ++i) delete[] argv[i];
    delete[] argv;

    // find if path argument if exists
    for(std::size_t i = 1; i < tokens.size(); ++i)
        if(tokens[i][0] != '-') {
            path = &tokens[i];
            break;
        }

    // print path to ostringstream and then pass oss to socket
    fatfs.print_all(oss, *path, is_details);
    sock::send_msg(sockfd, oss.str());
}

void pwd(int sockfd, fs::FatFS &fatfs) {
    if(fatfs.valid())
        sock::send_msg(sockfd, fatfs.pwd());
    else
        sock::send_msg(sockfd, "No filesystem");
}

}  // namespace fs
