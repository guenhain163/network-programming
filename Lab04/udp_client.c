/*UDP Echo Client*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define SERV_PORT 5550
#define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024
#define CONN "Connected!"

bool isValidIpAddress(char *ipAddress);

int main(int argc, char *argv[]) {
	if (argc != 3) {
        fprintf(stderr, "usage: %s hostname port\n", argv[0]);
        return 0;
    }

	char *serv_ip;
	int serv_port;

	if(isValidIpAddress(argv[1]) && atoi(argv[2])) {
		serv_ip = argv[1];
		serv_port = atoi(argv[2]);
    } else {
		printf("Error IP Address!\n");
		return 0;
	}

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received, sin_size;

	//Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serv_port);
	server_addr.sin_addr.s_addr = inet_addr(serv_ip);

	sin_size = sizeof(struct sockaddr);
	bytes_sent = sendto(client_sock, CONN, strlen(CONN), 0, (struct sockaddr *) &server_addr, sin_size);
	//Step 3: Communicate with server
	while(1) {
		// RECEIVED FROM SERVER
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}

		
		if (bytes_received) {
			buff[bytes_received] = '\0';
			printf("Reply from server: %s\n", buff);
		}

		// SEND TO SERVER
		printf("Insert string to send: ");
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);
		buff[strlen(buff)] = '\0';
		
		if (buff[0] == '\n')  {
			close(client_sock);
			printf("Exit\n");
			return 0;
		}

		// Send data to server
		bytes_sent = sendto(client_sock, buff, strlen(buff) - 1, 0, (struct sockaddr *) &server_addr, sin_size);
		if(bytes_sent < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
	}
		
	close(client_sock);
	return 0;
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}