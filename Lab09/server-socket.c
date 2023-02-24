#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define PORT 5500  // port number
#define BUFFER_SIZE 1024
#define BACKLOG 5 // Maximum number of connections in the queue
#define MAX_CLIENTS_CONNECTION 10 // Maximum number of connections per client

// Structure to hold the thread arguments
typedef struct
{
	int socket;
	char buffer[BUFFER_SIZE];
} thread_args;

// Remember to use -pthread when compiling this server's source code
void *connection_handler(void *);

int main()
{
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == -1)
	{
		perror("Socket initialisation failed");
		exit(EXIT_FAILURE);
	}
	else
		printf("Server socket created successfully\n");

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// bind the socket to the specified IP addr and port
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
	{
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Listen for incoming connections
	if (listen(server_socket, BACKLOG) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");

	// Accept client connections in a loop
	while (1)
	{
		printf("Listening...\n\n");

		// Accept a client connection
		struct sockaddr_in client_address;
		socklen_t client_address_len = sizeof(client_address);
		int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
		if (client_socket == -1)
		{
			printf("server acccept failed...\n");
			continue;
		}
		else
			printf("Server acccept the client...\n");

		printf("Connected to client %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

		// Create a thread to handle the client connection
		pthread_t thread;
		thread_args t_args;
		t_args.socket = client_socket;
		if (pthread_create(&thread, NULL, connection_handler, (void *)&t_args) != 0)
		{
			perror("Failed to create thread");
			close(client_socket);
			continue;
		}

		puts("Handler assigned");
	}

	close(server_socket);

	return 0;
}

void *connection_handler(void *client_socket)
{
	// Extract the socket and buffer from the thread arguments
	thread_args *t_args = (thread_args *)client_socket;
	int socket = t_args->socket;
	char *buffer = t_args->buffer;

	// Receive messages from the client
	while (1)
	{
		memset(buffer, 0, BUFFER_SIZE); // clear the buffer
		int bytes_received = recv(socket, buffer, BUFFER_SIZE - 1, 0);
		if (bytes_received == 0 || bytes_received == -1)
		{ // connection closed or error
			break;
		}

		// Check if the message has only letters
		int has_letters = 1;
		for (int i = 0; i < strlen(buffer); i++)
		{
			if (!isalpha(buffer[i]))
			{
				has_letters = 0;
				break;
			}
		}

		// Convert the message to uppercase and send it back to the client
		// or send an error message if the message has digits or special characters
		if (has_letters)
		{
			for (int i = 0; i < strlen(buffer); i++)
			{
				buffer[i] = toupper(buffer[i]);
			}
			send(socket, buffer, strlen(buffer), 0);
		}
		else
		{
			char *error_message = "Wrong text format\n";
			send(socket, error_message, strlen(error_message), 0);
		}

		// Close the connection if the string "q" or "Q" is received
		if (strcmp(buffer, "q") == 0 || strcmp(buffer, "Q") == 0)
		{
			break;
		}
	}

	// Close the socket
	close(socket);

	return 0;
}
