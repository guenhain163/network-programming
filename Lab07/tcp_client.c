#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // for close
#include "file_tranfer.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 1024

bool isValidIpAddress(char *ipAddress);

void getPath(char *path);

void getFilename(char *path, char *filename);

bool checkFileIsExist(char *filename);

int main(int argc, char *argv[]) {
	if (argc != 3) {
        fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
        return 0;
    }

	char *serv_addr;
	int serv_port;

	// Get information server name
	if(isValidIpAddress(argv[1]) && atoi(argv[2])) {
		serv_addr = argv[1];
		serv_port = atoi(argv[2]);
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

	// Insert filename
	char filename[BUFF_SIZE], path[BUFF_SIZE];
	getPath(path);
	getFilename(path, filename);
	if (!checkFileIsExist(path)) {
		perror("\nError");
		close(client_sock);
        return 0;
	}

	//Step 4: Communicate with server
	if (!sendData(client_sock, filename, sizeof(filename))) {
		perror("\nError sending filename!\n");
	} else {
		if (!sendFile(path, client_sock)) {
			perror("\nError sending file!\n");
		}

		if (!readData(client_sock, filename, BUFF_SIZE)) {
			perror("\nError reading filename!\n");
		}

		if (!writeFile(client_sock, filename)) {
			perror("\nError reading file!\n");
		}
		
		// Receive echo reply
		char buff[BUFF_SIZE];
		if (!readData(client_sock, buff, BUFF_SIZE)) {
			perror("\nError");
		}
		printf("\nReply from server: %s\n", buff);
		
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

void getPath(char *path)
{
	printf("Insert path and file: ");
	fgets(path, BUFF_SIZE, stdin);
	path[strlen(path) - 1] = '\0';
}

void getFilename(char *path, char *filename)
{
	int i, len = strlen(path);
	int index = 0;
	for (i = 0; i < len; i++) {
		if (path[i] == '/')
			index = i;
	}
	index++;

	strncpy(filename, path + index, len - index);
	filename[len - index] = '\0';
}

bool checkFileIsExist(char *filename)
{
	FILE *fp = fopen(filename, "r");
    bool is_exist = false;

    if (fp != NULL) {
        is_exist = true;
        fclose(fp); // close the file
    }

    return is_exist;
}
