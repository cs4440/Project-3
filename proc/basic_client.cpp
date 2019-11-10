#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFLEN 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Hello world";
    char buf[BUFLEN] = {0};

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR opening socket\n");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        error("ERROR Invalid address / Address not supported\n");
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting\n");
    }

    printf("String sent: %s\n", message);
    if(write(sockfd, message, strlen(message)) < 0) {
        error("ERROR writing to socket");
    }
    if(read(sockfd, buf, BUFLEN) < 0) {
        error("ERROR reading from socket");
    }
    printf("String recieved: %s\n", buf);

    return EXIT_SUCCESS;
}