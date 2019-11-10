#include <netinet/in.h>
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
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd;
    int opt = 1;
    int addrlen = sizeof(cli_addr);
    char buf[BUFLEN] = {0};
    char revbuf[BUFLEN] = {0};

    // Creating socket file descriptor
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR opening socket\n");
    }

    // Forcefully attaching socket to the port 8080
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                  sizeof(opt))) {
        error("ERROR on setsockopt\n");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding\n");
    }
    if(listen(sockfd, 3) < 0) {
        error("ERROR on listen\n");
    }
    if((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                           (socklen_t *)&addrlen)) < 0) {
        error("ERROR on accept\n");
    }

    if(read(newsockfd, buf, BUFLEN) < 0) {
        error("ERROR reading from socket\n");
    }

    // Reverse string
    int i;
    int j = 0;
    int len = strlen(buf);
    for(i = len - 1; i >= 0; i--) {
        revbuf[j++] = buf[i];
    }
    revbuf[i] = '\0';

    if(write(newsockfd, revbuf, strlen(revbuf)) < 0) {
        error("ERROR writing to socket\n");
    }

    return EXIT_SUCCESS;
}