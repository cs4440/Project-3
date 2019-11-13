#include <cstdlib>   // atoi(), rand()
#include <iostream>  // iostream
#include <sstream>   // stringstream
#include <string>    // string
#include "../include/socket.h"

std::string get_rand_disk_indices(int cyl, int sec);
std::string get_rand_128bytes_data();

int main(int argc, char* argv[]) {
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

        int num = 0, seed = 0;
        std::cout << "Enter N requests and random seed number:\n[N] [S]: ";

        std::cin >> num >> seed;
        std::cin.ignore(1024, '\n');
        std::cout << std::endl;

        sockfd = client.sockfd();

        // read server welcome message
        sock::read_msg(sockfd, server_msg);
        std::cout << server_msg << std::endl;

        // send I command to get geometry info
        int cyl = 0, sec = 0;
        std::stringstream ss;
        std::cout << "I" << std::endl;
        sock::send_msg(sockfd, "I");
        sock::read_msg(sockfd, server_msg);
        ss << server_msg;
        ss >> cyl >> sec;

        // create disk if there is no disk
        if(cyl == 0) {
            cyl = 5;
            sec = 10;
            std::string create =
                "C " + std::to_string(cyl) + " " + std::to_string(sec);
            std::cout << "Server has no disk. Creating with: " + create
                      << std::endl;
            sock::send_msg(sockfd, create);
            sock::read_msg(sockfd, server_msg);
        }
        std::cout << server_msg << std::endl;

        // start num random requests
        srand(seed);
        int random = -1;
        std::string rand_msg, server_msg;
        for(int i = 0; i < num; ++i) {
            random = rand() % 2;

            switch(random) {
                case 0:
                    rand_msg.clear();
                    rand_msg = "R " + get_rand_disk_indices(cyl, sec);
                    std::cout << "R" << std::endl;
                    sock::send_msg(sockfd, rand_msg);
                    sock::read_msg(sockfd, server_msg);
                    break;
                case 1:
                    rand_msg.clear();
                    rand_msg = "W " + get_rand_disk_indices(cyl, sec) + " " +
                               get_rand_128bytes_data();
                    std::cout << "W" << std::endl;
                    sock::send_msg(sockfd, rand_msg);
                    sock::read_msg(sockfd, server_msg);
                    break;
                default:
                    std::cout << "ERROR Random" << std::endl;
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

std::string get_rand_disk_indices(int cyl, int sec) {
    return std::string(std::to_string(rand() % cyl) + " " +
                       std::to_string(rand() % sec));
}

std::string get_rand_128bytes_data() {
    std::string str(130, '"');
    for(int i = 1; i < 129; ++i) str[i] = rand() % 75 + 49;

    return str;
}
