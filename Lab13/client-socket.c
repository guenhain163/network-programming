#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    int sockfd, key;
    char buf[1024];
    struct sockaddr_in servaddr;
    char *IP = argv[1];
    int PORT = atoi(argv[2]);

    // Create a socket and connect to the server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    while (1) {
        // Receive the key from the server
        recv(sockfd, buf, 1024, 0);
        key = atoi(buf);
        printf("Received key: %d\n", key);
        // Encrypt/decrypt the message using the Caesar cipher
        printf("Enter a message: ");
        fgets(buf, sizeof(buf), stdin);

        for (int i = 0; i < strlen(buf); i++) {
            if (isalpha(buf[i])) {
                buf[i] = (isupper(buf[i])) ? ((buf[i] - 'A' + key) % 26) + 'A' : ((buf[i] - 'a' + key) % 26) + 'a';
            }
        }

        // Send the encrypted message to the server
        send(sockfd, buf, strlen(buf), 0);
        // Receive the decrypted message from the server
        recv(sockfd, buf, 1024, 0);
        printf("Received message: %s\n", buf);
    }
    close(sockfd);
    return 0;
}
