#ifndef SOCKET_H
#define SOCKET_H

#include <arpa/inet.h>   // htons()
#include <netinet/in.h>  // struct sockaddr_in
#include <sys/socket.h>  // socket()
#include <unistd.h>      // close()
#include <cstring>       // memset
#include <stdexcept>     // std::exception
#include <string>        // std::string

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
    socklen_t _addrlen;
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

// send to socket, blocking mode, throws on error
void send_msg(int sockfd, const std::string &msg);
void send_msg(int sockfd, char *msg, std::size_t sz);

// receive from socket, blocking mode, throws on error
void recv_msg(int sockfd, std::string &msg);

// throw from read, recv, write, send return error values for blocking mode
void throw_socket_io(int value);

}  // namespace sock

#endif  // SOCKET_H
