#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#define BUFSIZE 1024

/* Ceasar Cipher encryption/decryption */
void ceasar_encrypt(char *plaintext, int key) {
    int i;
    for (i = 0; i < strlen(plaintext); i++) {
        if (plaintext[i] >= 'a' && plaintext[i] <= 'z') {
            plaintext[i] = ((plaintext[i] - 'a') + key) % 26 + 'a';
        } else if (plaintext[i] >= 'A' && plaintext[i] <= 'Z') {
            plaintext[i] = ((plaintext[i] - 'A') + key) % 26 + 'A';
        }
    }
}

void ceasar_decrypt(char *ciphertext, int key) {
    int i;
    for (i = 0; i < strlen(ciphertext); i++) {
        if (ciphertext[i] >= 'a' && ciphertext[i] <= 'z') {
            ciphertext[i] = ((ciphertext[i] - 'a') - key + 26) % 26 + 'a';
        } else if (ciphertext[i] >= 'A' && ciphertext[i] <= 'Z') {
            ciphertext[i] = ((ciphertext[i] - 'A') - key + 26) % 26 + 'A';
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, filefd, n, opcode, key, length, maxfdp1, nready, i;
    char buffer[BUFSIZE], filename[256], *tempfile;
    struct sockaddr_in serv_addr;
    fd_set rset, wset;
    struct timeval timeout;

    /* Check for valid command line arguments */
    if (argc < 4) {
        fprintf(stderr, "usage: %s <IP address> <port> <file>\n", argv[0]);
        exit(1);
    }

    /* Open the file to be encrypted/decrypted */
    filefd = open(argv[3], O_RDONLY);
    if (filefd < 0) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        exit(1);
    }

    /* Create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error creating socket: %", strerror(errno));
exit(1);
}
/* Initialize the server address */
memset(&serv_addr, 0, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(atoi(argv[2]));
if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Error initializing server address: %s\n", strerror(errno));
    exit(1);
}

/* Connect to the server */
if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
    exit(1);
}

/* Get the encryption/decryption opcode, key, and filename from the user */
printf("Enter opcode (0 for encryption, 1 for decryption): ");
scanf("%d", &opcode);
printf("Enter key: ");
scanf("%d", &key);
printf("Enter file name: ");
scanf("%s", filename);

/* Send the request message to the server */
buffer[0] = (char) opcode;
*((unsigned short *) (buffer + 1)) = htons((unsigned short) key);
strncpy(buffer + 3, filename, BUFSIZE - 3);
n = send(sockfd, buffer, strlen(filename) + 3, 0);
if (n < 0) {
    fprintf(stderr, "Error sending request message: %s\n", strerror(errno));
    exit(1);
}

/* Send the file data to the server */
while (1) {
    memset(buffer, 0, BUFSIZE);
    n = read(filefd, buffer, BUFSIZE);
    if (n <= 0) break;

    *((unsigned short *) (buffer + 1)) = htons((unsigned short) n);
    n = send(sockfd, buffer, n + 3, 0);
    if (n < 0) {
        fprintf(stderr, "Error sending file data: %s\n", strerror(errno));
        exit(1);
    }
}

/* Send the end of file message to the server */
buffer[0] = (char) 2;
*((unsigned short *) (buffer + 1)) = htons(0);
n = send(sockfd, buffer, 3, 0);
if (n < 0) {
    fprintf(stderr, "Error sending end of file message: %s\n", strerror(errno));
    exit(1);
}

/* Receive the resulting file from the server */
tempfile = "tempfile.txt";
filefd = open(tempfile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
if (filefd < 0) {
    fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
exit(1);
}

/* Set up select for file reading, writing and timeout */
FD_ZERO(&rset);
FD_SET(sockfd, &rset);
maxfdp1 = sockfd + 1;
timeout.tv_sec = 30;
timeout.tv_usec = 0;

while (1) {
    wset = rset;
    nready = select(maxfdp1, &rset, &wset, NULL, &timeout);
    if (nready == 0) {
        printf("Server did not respond after 30 seconds");
        exit(1);
    }
    if (FD_ISSET(sockfd, &rset)) {
        n = recv(sockfd, buffer, BUFSIZE, 0);
        if (n <= 0) {
            printf("Error reading from socket");
            exit(1);
        }
        opcode = (int) buffer[0];
        length = ntohs(*((unsigned short *) (buffer + 1)));
        /*Check for operation and handle it accordingly*/
        switch(opcode)
        {
            case 2: /* File Data transfer */
                write(filefd, buffer + 3, length);
                break;
            case 3:/* Error */
                printf("Error: %s", buffer + 3);
                exit(1);
                break;
            default:
                printf("Invalid Opcode received");
                exit(1);
        }
        if(opcode == 2 && length == 0) break;
    }
}

/* Decrypt/Encrypt the file based on the request */
close(filefd);
filefd = open(tempfile, O_RDONLY);
while (1) {
    memset(buffer, 0, BUFSIZE);
    n = read(filefd, buffer, BUFSIZE);
    if (n <= 0) break;

    if (opcode == 0) {
        ceasar_encrypt(buffer, key);
    } else {
        ceasar_decrypt(buffer, key);
    }
    n = write(1, buffer, n);
    if (n < 0) {
        fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
        exit(1);
    }
}
close(filefd);

/* Close the socket and delete the temporary file */
close(sockfd);
unlink(tempfile);

return 0;
}
