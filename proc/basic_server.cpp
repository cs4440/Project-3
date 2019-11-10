#include <netinet/in.h>
#include <pthread.h>
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

void *connection_handler(void *socket_desc) {
    int sockfd = *(int *)socket_desc;
    char buf[BUFLEN] = {0};
    char revbuf[BUFLEN] = {0};

    if(read(sockfd, buf, BUFLEN) < 0) {
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

    if(write(sockfd, revbuf, strlen(revbuf)) < 0) {
        error("ERROR writing to socket\n");
    }

    free(socket_desc);

    return 0;
}

int main(void) {
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd;
    int *thsockfd;
    int opt = 1;
    int addrlen = sizeof(cli_addr);

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

    while((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                              (socklen_t *)&addrlen))) {
        pthread_t sniffer_thread;
        thsockfd = (int *)malloc(1);
        *thsockfd = newsockfd;

        if(newsockfd < 0) {
            error("ERROR on accept\n");
        }

        if(pthread_create(&sniffer_thread, NULL, connection_handler,
                          (void *)thsockfd)) {
            error("ERROR on creating thread");
        }
        pthread_join(sniffer_thread, NULL);
    }

    return EXIT_SUCCESS;
}