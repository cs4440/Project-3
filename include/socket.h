#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>   // htons()
#include <netinet/in.h>  // struct sockaddr_in
#include <sys/socket.h>  // socket()
#include <unistd.h>      // close()
#include <cstring>       // memset
#include <stdexcept>     // std::exception
#include <string>        // std::string
#include "utils.h"       // char_to_int(), int_to_char()

namespace sock {

enum { PORT = 8000, BUFLEN = 1024 };

class Socket {
public:
    Socket(int port = PORT);
    virtual ~Socket();

    int sockfd();  // get sockfd
    int port();    // get port

    void close_socket();      // close connection
    void set_port(int port);  // set port

protected:
    struct sockaddr_in _serv_addr;
    int _sockfd;
    int _port;
};

class Server : public Socket {
public:
    Server(int port = PORT);

    void start();
    void stop();
    int accept_connection();  // return new sockfd for incoming connection

private:
    struct sockaddr_in _cli_addr;
    int _opt;
    int _addrlen;
};

class Client : public Socket {
public:
    Client(std::string host = "localhost", int port = PORT);

    void start();
    void stop();
    void set_host(std::string host);

private:
    std::string _host;
};

// HELPER FUNCTIONS

void send_msg(int sockfd, const std::string &msg);     // send msg to socket
void send_msg(int sockfd, char *msg, std::size_t sz);  // send msg to socket
// read from socket, throw exception on disconnected
void read_msg(int sockfd, std::string &msg);

}  // namespace sock

#endif  // SOCKET_H
