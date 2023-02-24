#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // for close

#define SERVER_ADDR "127.0.0.1"
#define SOCKET_ERROR -1
#define SERVER_PORT 5550
#define BUFF_SIZE 1024

bool isValidIpAddress(char *ipAddress);


int main(int argc, char *argv[]) {
	if (argc != 5) {
        fprintf(stderr, "usage: %s <hostname> <port> <username> <password>\n", argv[0]);
        return 0;
    }

	char *serv_addr;
	int serv_port;
	char *username;
    char *password;

	// Get information server name
	if(isValidIpAddress(argv[1]) && atoi(argv[2])) {
		serv_addr = argv[1];
		serv_port = atoi(argv[2]);
		username = argv[3];
        password = argv[4];
    } else {
		printf("Error IP Address!\n");
		return 0;
	}

	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serv_port);
	server_addr.sin_addr.s_addr = inet_addr(serv_addr);
	
	//Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
		printf("\nError!Can not connect to sever! Client exit imediately!\n");
		return 0;
	}

	//Step 4: Communicate with server
	// send(client_sock, strlen(username), sizeof(username), 0);
	
	int len = strlen(username);
	send(client_sock, &len, sizeof(len), 0);
	send(client_sock, username, strlen(username), 0);
	len = strlen(password);
	send(client_sock, &len, sizeof(len), 0);
	send(client_sock, password, strlen(password), 0);
	
	// if (!send(client_sock, username, strlen(username), 0) || !send(client_sock, password, strlen(password), 0) ) {
	// 	perror("\nError: ");
    //     exit(1);
	// }

	// Check the login result
    int login_result;
	if (!recv(client_sock, &login_result, sizeof(login_result), 0)) {
		perror("\nError: recv");
        exit(1);
	}
	
    if (login_result == 0) {
        printf("Login failed\n");
        exit(1);
    }

    // Login successful: join the group chat
    printf("\nLogged in as %s\n\n", username);

    // Create a child process to receive chat messages
    pid_t pid = fork();
    if (pid < 0) {
        perror("Error forking process");
        exit(1);
    }
	
	if (pid > 0) {
        // Child process: receive chat messages
        while (1) {
            char message[BUFF_SIZE];
			int bytes_received = 0;
			if ((bytes_received = recv(client_sock, message, BUFF_SIZE, 0)) <= 0) {
				break;
			}
			message[bytes_received] = '\0';
			printf("%s\n", message);
        }
    } else {
        // Parent process: send chat messages
        while (1) {
            char message[BUFF_SIZE];
			printf("%s: ", username);
            fgets(message, BUFF_SIZE, stdin);
			message[strlen(message) - 1] = '\0';
			if (!send(client_sock, message, strlen(message), 0)) {
				break;
			}
			printf("\n");
        }
    }
	

	//Step 5: Close socket
	close(client_sock);
	return 0;
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}
