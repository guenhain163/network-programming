#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include "datatypes.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 8192

enum STATUS {
    ERROR_CONNECT,
    SUCCESS,
    USER_EXIST,
    USER_NOT_EXIST,
    PASS_INCORRECT,
    IS_BLOCKED,
    LOGIN_TOO_MUCH,
    LOGOUT
};

int isValidIpAddress(char *ipAddress);

int sendData(int client_sock, int type);

int recvData(int client_sock, int *data);

int main(int argc, char *argv[]) {
	if (argc != 3) {
        fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
        return 0;
    }

	char *serv_addr;
	int serv_port;

	if(isValidIpAddress(argv[1]) && atoi(argv[2])) {
		serv_addr = argv[1];
		serv_port = atoi(argv[2]);
    } else {
		printf("Error IP Address!\n");
		return 0;
	}

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
    int recv_data;
	
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
    int input = 0;			
	while(1) {
		if (!sendData(client_sock, input)) {
            break;
        }

		//receive echo reply
		if (!recvData(client_sock, &recv_data)) {
            break;
        }

        input = 0;
        switch (recv_data) {
            case ERROR_CONNECT: 
                printf("Connection closed!\n");
                break;
            case SUCCESS:
                input = 2;
                printf("Login is successful!");
                break;
            case USER_EXIST: 
                input = 1;
                break;
            case USER_NOT_EXIST:
                printf("Account does not exist\n");
                break;
            case PASS_INCORRECT:
                input = 1;
                printf("Password is not correct. Please try again!\n");
                break;
            case IS_BLOCKED:
                printf("Account is blocked or inactive\n");
                break;
            case LOGIN_TOO_MUCH:
                printf("Account is blocked!\n");
                break;
            case LOGOUT:
                bytes_received = recv(client_sock, buff, sizeof(buff), 0);
                if (bytes_received <= 0) {
                    printf("\nError!Cannot receive data from sever!\n");
                    return 0;
                }
                printf("Goodbye %s\n", buff);
                break;
            default:
                break;
        }
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

// 0 - user
// 1 - pass
int sendData(int client_sock, int type)
{
    char buff[BUFF_SIZE];
    int msg_len, bytes_sent;

    //send message
    if (type == 1) {
        printf("Please enter the login password: ");
    } else if (type == 2) {
        printf("\n\nEnter \"Bye\" to logout: ");
    } else {
        printf("\n\nPlease enter the login username: ");
    }

	memset(buff,'\0',(strlen(buff) + 1));
	fgets(buff, BUFF_SIZE, stdin);		
	buff[strlen(buff) - 1] = '\0';

    msg_len = strlen(buff);    
	if (msg_len == 0) return 0;
		
	bytes_sent = send(client_sock, buff, msg_len, 0);
	if (bytes_sent <= 0) {
        printf("\nConnection closed!\n");
		return 0;
	}

    return 1;
}

int recvData(int client_sock, int *data)
{
    uint32_t buff;
    int msg_len, bytes_received;

    bytes_received = recv(client_sock, &buff, sizeof(buff), 0);
	if (bytes_received <= 0) {
		printf("\nError!Cannot receive data from sever!\n");
		return 0;
	}
    
    *data = ntohl(buff);
    return 1;
}