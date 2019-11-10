#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFLEN 1024
#define TOKLEN 8

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void connection_handler(int sockfd) {
    char buf[BUFLEN] = {0};
    char *token = NULL;
    char *args[TOKLEN] = {0};
    int pos = 0;

    if(read(sockfd, buf, BUFLEN) < 0) {
        error("ERROR reading from socket\n");
    }

    token = strtok(buf, " ");
    while(token != NULL) {
        args[pos++] = token;
        token = strtok(NULL, " ");
    }
    args[pos] = NULL;

    dup2(sockfd, STDOUT_FILENO);

    execvp("ls", args);

    free(token);
}

int main(void) {
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd, pid;
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
    if(listen(sockfd, 5) < 0) {
        error("ERROR on listen\n");
    }

    while(1) {
        if((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                               (socklen_t *)&addrlen)) < 0) {
            error("ERROR on accept\n");
        }
        pid = fork();
        if(pid < 0) {
            error("ERROR on fork");
        }
        if(pid == 0) {
            close(sockfd);
            connection_handler(newsockfd);
            exit(EXIT_SUCCESS);
        } else {
            close(newsockfd);
        }
    }

    return EXIT_SUCCESS;
}
