#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define BUF_SIZE 1024

struct client
{
    int fd;
    struct sockaddr_in addr;
} clients[MAX_CLIENTS];

int nclients = 0;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUF_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, maxfd;
    fd_set readfds;

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, MAX_CLIENTS);

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        maxfd = sockfd;

        for (i = 0; i < nclients; i++)
        {
            FD_SET(clients[i].fd, &readfds);
            if (clients[i].fd > maxfd)
                maxfd = clients[i].fd;
        }

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0)
            error("ERROR on select");

        if (FD_ISSET(sockfd, &readfds))
        {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0)
                error("ERROR on accept");

            clients[nclients].fd = newsockfd;
            clients[nclients].addr = cli_addr;
            nclients++;
        }

        for (i = 0; i < nclients; i++)
        {
            if (FD_ISSET(clients[i].fd, &readfds))
            {
                bzero(buffer, BUF_SIZE);
                n = read(clients[i].fd, buffer, BUF_SIZE - 1);
                if (n < 0)
                    error("ERROR reading from socket");
                else if (n == 0)
                {
                    close(clients[i].fd);
                    clients[i] = clients[nclients - 1];
                    nclients--;
                    i--;
                    continue;
                }

                // Process the received data
                int opcode = buffer[0];
                int length = buffer[1] << 8 | buffer[2];
                char payload[length];
                memcpy(payload, buffer + 3, length);

                if (opcode == 0)
                {
                    // Encryption requested
                    int key = atoi(payload);
                    // Perform Caesar encryption on the temporary file
                    // ...
                }
                else if (opcode == 1)
                {
                    // Decryption requested
                    int key = atoi(payload);
                    // Perform Caesar decryption on the temporary file
                    // ...
                }
                else if (opcode == 2)
                {
                    if (length > 0)
                    {
                        // Write payload to temporary file
                        // ...
                    }
                    else
                    {
                        // File transfer complete, send the resulting file to the client
                        // ...
                    }
                }
                else if (opcode == 3)
                {
                    // Error reporting
                    // ...
                }
            }
        }

        // Close the socket and exit
        close(sockfd);
        return 0;
    }
}