#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <string>

namespace sock {

enum { PORT = 8000 };

class Socket {
public:
    Socket(int port = PORT);
    virtual ~Socket();

    int sockfd();  // get sockfd
    int port();    // get port

    void close_socket();  // close connection
    void set_port(int port);

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
    Client(std::string host);

    void start();
    void stop();
    void set_host(std::string host);

private:
    std::string _host;
};

// HELPER FUNCTIONS
void int_to_char(int n, char *str);   // convert int to char[4]
void char_to_int(char *str, int &n);  // convert char[4] to int

}  // namespace sock

#endif  // SERVER_H
