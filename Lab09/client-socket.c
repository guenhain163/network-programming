#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define PORT 5500        // port number
#define ADD_SERV "127.0.0.1"
#define BUFFER_SIZE 1024 // maximum number of bytes that can be received at once

int main(int argc, const char *argv[])
{
    // Create a socket
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Specify an address for the socket
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(ADD_SERV);

    // Check for connection_status
    if (connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        printf("The connection has error\n\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");

    // Read messages from the keyboard and send them to the server
    char buffer[BUFFER_SIZE];
    int bytes_sent = 0;
    while (1)
    {

        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        int length = strlen(buffer);
        if (buffer[length - 1] == '\n')
        {
            buffer[length - 1] = '\0';
        }

        // Stop sending and close the connection if the user enters the string "q" or "Q"
        if (strcmp(buffer, "q") == 0 || strcmp(buffer, "Q") == 0)
        {
            break;
        }

        // Send a message
        bytes_sent += send(network_socket, buffer, strlen(buffer), 0);

        // Receive and display the response from the server
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(network_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == 0 || bytes_received == -1)
        { // connection closed or error
            break;
        }
        printf("Response: %s\n\n", buffer);
    }

    // Close the socket
    close(network_socket);

    printf("Sent %d bytes to the server.\n", bytes_sent);

    return 0;
}
