#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#define PORT 5550
#define BACKLOG 20
#define BUFF_SIZE 1024
#define MAX_CLIENTS 10

// Structure to store client information
typedef struct
{
	int sockfd;
	char username[BUFF_SIZE];
} Client;

// Array to store all connected clients
Client clients[MAX_CLIENTS];
int num_clients = 0;

/* Handler process signal*/
void sig_chld(int signo);

void clear();

void handle_client(int client_fd);

int checkUsername(char *username);

void addClient(int sockfd, char *username);

void removeClient(int sockfd);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		return 0;
	}

	int port;
	port = atoi(argv[1]);

	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server;	/* server's address information */
	struct sockaddr_in client;	/* client's address information */
	pid_t pid;
	int sin_size;

	// Create a socket
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error creating socket");
		return 1;
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Error binding socket");
		exit(1);
	}

	if (listen(listen_sock, BACKLOG) == -1)
	{
		perror("Error listiing socket");
		exit(1);
	}

	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while (1)
	{
		// Accept incoming connections
		sin_size = sizeof(struct sockaddr_in);
		conn_sock = accept(listen_sock, (struct sockaddr *)&client, (socklen_t *)&sin_size);

		if (conn_sock == -1)
		{
			if (errno == EINTR)
				continue;
			else
			{
				perror("\nError: ");
				return 0;
			}
		}

		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();

		if (pid < 0)
		{
			perror("Error forking process");
			close(conn_sock);
			continue;
		}

		/* fork() is called in child process */
		if (pid == 0)
		{
			// Child process: handle the connection
			close(listen_sock);
			printf("\nYou got a connection from %c\n", *inet_ntoa(client.sin_addr)); /* prints client's IP */

			// Check the client's login information
			handle_client(conn_sock);

			exit(0);
		}

		/* The parent closes the connected socket since the child handles the new client */
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	/* Wait the child process terminate */
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("\nChild %d terminated\n", pid);
}

void handle_client(int client_fd)
{
	int bytes_received;
	char username[BUFF_SIZE], password[BUFF_SIZE];
	char buf_user[BUFF_SIZE], buf_pass[BUFF_SIZE];

	// Receive login information from the client
	recv(client_fd, &bytes_received, sizeof(bytes_received), 0);
	recv(client_fd, username, bytes_received, 0);

	recv(client_fd, &bytes_received, sizeof(bytes_received), 0);
	recv(client_fd, password, bytes_received, 0);

	// Validate login information by reading from the file "taikhoan.txt"
	FILE *fp = fopen("account.txt", "r");
	if (fp == NULL)
	{
		perror("Error opening file");
		return;
	}

	int valid = 0;
	while (fscanf(fp, "%s %s\n", buf_user, buf_pass) != EOF)
	{
		if (strcmp(buf_user, username) == 0 && strcmp(buf_pass, password) == 0)
		{
			valid = 1;
			if (!checkUsername(username))
			{
				addClient(client_fd, username);
				num_clients++;
			}
			break;
		}
		printf("\n");
	}
	fclose(fp);

	// Send a confirmation message to the client
	send(client_fd, &valid, sizeof(int), 0);

	// If login is successful, join the group chat
	if (valid)
	{

		// Open the file "groupchat.txt" for appending
		fp = fopen("groupchat.txt", "r");
		if (fp == NULL)
		{
			perror("Error opening file");
			return;
		}

		// Send the chat history to the client
		char line[BUFF_SIZE];
		// while (fgets(line, BUFF_SIZE, fp))
		// {
		// 	send(client_fd, line, strlen(line), 0);
		// }
		while (fscanf(fp, "%[^\n]\n", line) != EOF)
		{
			strcat(line, "\n");
			send(client_fd, line, strlen(line), 0);
		}
		fclose(fp);

		// Continuously receive chat messages from the client and write them to the file
		while (1)
		{
			char message[BUFF_SIZE];
			bytes_received = recv(client_fd, message, BUFF_SIZE, 0);
			if (bytes_received > 0)
			{
				message[bytes_received] = '\0';
				// Store the message in the chat history file
				fp = fopen("groupchat.txt", "a");
				fprintf(fp, "%s: %s\n", username, message);
				fclose(fp);

				// Forward the message to all other clients
				for (int i = 0; i < num_clients; i++)
				{
					if (clients[i].sockfd != client_fd)
					{
						char temp_msg[BUFF_SIZE];
						strcpy(temp_msg, username);
						strcat(temp_msg, ": ");
						strcat(temp_msg, message);
						send(clients[i].sockfd, temp_msg, strlen(temp_msg), 0);
					}
				}
			} else {
				break;
			}
		}

		// Close the file
		fclose(fp);
	}
	else
	{
		// If login fails, close the connection
		close(client_fd);
	}
}

// Function to check if a username is already taken
int checkUsername(char *username)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (strcmp(clients[i].username, username) == 0)
		{
			return 1;
		}
	}
	return 0;
}

// Function to add a new client to the list of connected clients
void addClient(int sockfd, char *username)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (clients[i].sockfd == 0)
		{
			clients[i].sockfd = sockfd;
			strcpy(clients[i].username, username);
			break;
		}
	}
}

// Function to remove a client from the list of connected clients
void removeClient(int sockfd)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (clients[i].sockfd == sockfd)
		{
			clients[i].sockfd = 0;
			strcpy(clients[i].username, "");
			break;
		}
	}
}

void clear()
{
	int c;

	while ((c = getchar()) != '\n' && c != EOF)
	{
	}
}