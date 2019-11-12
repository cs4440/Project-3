#include "../include/socket.h"

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

Client::Client(std::string host, int port) : Socket(port), _host(host) {}

void Client::start() {
    // set server address options
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

// send msg to socket
void send_msg(int sockfd, const std::string &msg) {
    char msg_len[4] = {0};
    int msg_size = 0;

    // get msg size and send message length
    msg_size = msg.size();                  // get size
    utils::int_to_char(msg_size, msg_len);  // convert to 4 byte char
    write(sockfd, msg_len, 4);              // send message length

    // send message through socket
    write(sockfd, msg.c_str(), msg.size());
}

void send_msg(int sockfd, char *msg, std::size_t sz) {
    char msg_len[4] = {0};

    // get msg size and send message length
    utils::int_to_char(sz, msg_len);  // convert to 4 byte char
    write(sockfd, msg_len, 4);        // send message length

    // send message through socket
    write(sockfd, msg, sz);
}

// read from socket and return by ref to msg
void read_msg(int sockfd, std::string &msg) {
    char msg_len[4] = {0}, buf[BUFLEN] = {0};
    int msg_size = 0, bytes = 0, totalbytes = 0;
    msg.clear();

    bytes = read(sockfd, msg_len, 4);  // read message size
    if(bytes > 0) {
        utils::char_to_int(msg_len, msg_size);  // convert 4 byte char to int

        // keep reading from socket until msg_sze is reached
        while(bytes > 0 && totalbytes < msg_size) {
            bytes = read(sockfd, buf, BUFLEN - 1);
            if(bytes) {
                buf[bytes] = '\0';
                msg += buf;

                totalbytes += bytes;
            } else
                throw std::runtime_error("Disconnected");
        }
        bytes = 0;
        totalbytes = 0;
    } else
        throw std::runtime_error("Disconnected");
}

}  // namespace sock
