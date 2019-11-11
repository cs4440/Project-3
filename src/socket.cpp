#include "../include/socket.h"
#include <iostream>

namespace sock {

Socket::Socket(int port) : _port(port) {
    memset((void *)&_serv_addr, 0, sizeof(_serv_addr));
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(_sockfd < 0) throw std::runtime_error("ERROR opening socket");
}

Socket::~Socket() {
    if(_sockfd > -1) close(_sockfd);
}

int Socket::sockfd() { return _sockfd; }

int Socket::port() { return _port; }

void Socket::close_socket() {
    if(_sockfd > -1) {
        close(_sockfd);
        _sockfd = -1;
    }
}

void Socket::set_port(int port) { _port = port; }

Server::Server(int port) : Socket(port), _opt(1), _addrlen(sizeof(_cli_addr)) {
    memset((void *)&_cli_addr, 0, sizeof(_cli_addr));
}

void Server::start() {
    // Forcefully attaching socket to the port 8080
    if(setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &_opt,
                  sizeof(_opt)))
        throw std::runtime_error("ERROR on setsockopt for option " +
                                 std::to_string(_opt));

    // set server address options
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_addr.s_addr = INADDR_ANY;  // any address
    _serv_addr.sin_port = htons(_port);  // convert port to network byte order

    // bind server address and port to socket
    if(bind(_sockfd, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
        throw std::runtime_error("ERROR on binding on port " +
                                 std::to_string(_port));

    // mark socket to accept connection
    if(listen(_sockfd, 5) < 0) throw std::runtime_error("ERROR on listen");
}

void Server::stop() { close_socket(); }

int Server::accept_connection() {
    int _newsockfd =
        accept(_sockfd, (struct sockaddr *)&_cli_addr, (socklen_t *)&_addrlen);

    if(_newsockfd < 0) throw std::runtime_error("Error on accept connection");

    return _newsockfd;
}

Client::Client(std::string host) : Socket(), _host(host) {}

void Client::start() {
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, _host.c_str(), &_serv_addr.sin_addr) <= 0)
        std::runtime_error(
            "ERROR Invalid address / Address not supported on port " +
            std::to_string(_port));

    if(connect(_sockfd, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) <
       0) {
        std::runtime_error("ERROR connecting");
    }
}

void Client::stop() { close_socket(); }

void Client::set_host(std::string host) { _host = host; }

// HELPER FUNCTIONS
void int_to_char(int n, char *str) {
    str[0] = n >> 24 & 0xFF;
    str[1] = n >> 16 & 0xFF;
    str[2] = n >> 8 & 0xFF;
    str[3] = n & 0xFF;
}

void char_to_int(char *str, int &n) {
    n = ((unsigned char)str[0] << 24) | ((unsigned char)str[1] << 16) |
        ((unsigned char)str[2] << 8) | (unsigned char)str[3];
}

}  // namespace sock
