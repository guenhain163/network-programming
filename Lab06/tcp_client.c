#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "datatypes.h"

#define SOCKET_ERROR -1
#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 8192

void clear();

char *strlwr(char *s);

bool isValidIpAddress(char *ipAddress);

bool sendFilename(int client_sock, char *filename);

bool checkFilename(char *filename);

bool recvMessage(int client_sock, char *massage);

bool writeFile(int sockfd, char *filename);

bool readData(int sock, void *buf, int buflen);

bool readLong(int sock, long *value);

bool readFile(int sock, FILE *f);

int main(int argc, char *argv[]) {
	if (argc != 4) {
        fprintf(stderr, "usage: %s <hostname> <port> <filename>\n", argv[0]);
        return 0;
    }

	char *serv_addr;
    char *filename;
	int serv_port;

	if(isValidIpAddress(argv[1]) && atoi(argv[2]) && argv[3]) {
		serv_addr = argv[1];
		serv_port = atoi(argv[2]);
		filename = argv[3];
    } else {
		printf("Error IP Address!\n");
		return 0;
	}

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int bytes_sent, bytes_received;
    int recv_data;

	if (!checkFilename(filename)) {
        printf("Error: Wrong File Format!\n");
		return 0;
    }
	
	char path[BUFF_SIZE] = {0};
	strcat(path, getenv("HOME"));
	strcat(path, "/images/");
	strcat(path, filename);
	
	FILE *fp = fopen(path, "r");
	if (fp) {
		printf("File Already Exists!\n");
		fclose(fp);
		return 0;
	}
	
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
	if (!sendFilename(client_sock, filename)) {
		close(client_sock);
		return 0;
	}

	char message[BUFF_SIZE];
	if (!recvMessage(client_sock, message)) {
		close(client_sock);
		return 0;
	}

	if (strcmp(message, "Transferring\0") != 0) {
		printf ("%s\n", message);
	} else {
		if (!writeFile(client_sock, filename)) {
			printf("Error file!\n");
		} else 
			printf("File Transfer Is Completed...\n");
	}

	//Step 4: Close socket
	close(client_sock);
	return 0;
}

bool checkFilename(char *filename)
{
	char format[BUFF_SIZE];
	int begin = strlen(filename) - 4;
	strncpy(format, filename + begin, 4);
	format[4] = '\0';
	strlwr(format);

	if (strcmp(format, ".png") != 0 && strcmp(format, ".jpg") != 0 && strcmp(format, ".bmp") != 0) {
		return false;
	}

	return true;
}

void clear()
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

bool sendFilename(int client_sock, char *filename)
{
	int bytes_sent = send(client_sock, filename, strlen(filename), 0);
	if(bytes_sent <= 0){
		return false;
	}
	return true;
}

bool writeFile(int sockfd, char *filename)
{
	char path[BUFF_SIZE] = {0};
	
	strcat(path, getenv("HOME"));
	strcat(path, "/images/");
	strcat(path, filename);
	
	FILE *filehandle = fopen(path, "wb+");
	if (filehandle != NULL)
	{
		bool ok = readFile(sockfd, filehandle);
		fclose(filehandle);
	
		if (!ok) {
			remove(path);
			return false;
		}
	}

	return true;
}

bool recvMessage(int client_sock, char *message)
{
	char buff[BUFF_SIZE];
	int bytes_received;
    bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0); //blocking
	if (bytes_received <= 0){
		return false;
	} else {
		buff[bytes_received] = '\0';
        strcpy(message, buff);
	}
	return true;
}

char *strlwr(char *s)
{
    char *tmp = s;

    for (; *tmp; ++tmp)
    {
        *tmp = tolower((unsigned char)*tmp);
    }

    return s;
}

bool readData(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = recv(sock, pbuf, buflen, 0);
        if (num == SOCKET_ERROR || num == 0)
        {
            return false;
        }

        pbuf += num;
        buflen -= num;
    }

    return true;
}

bool readLong(int sock, long *value)
{
    if (!readData(sock, value, sizeof(value)))
        return false;
    *value = ntohl(*value);
    return true;
}

bool readFile(int sock, FILE *f)
{
    long filesize;
    if (!readLong(sock, &filesize))
        return false;

    if (filesize > 0)
    {
        char buffer[BUFF_SIZE];
        do {
            int num = filesize < sizeof(buffer) ? filesize : sizeof(buffer);
            if (!readData(sock, buffer, num))
                return false;
            int offset = 0;
            do {
                size_t written = fwrite(&buffer[offset], 1, num - offset, f);
                if (written < 1)
                    return false;
                offset += written;
            } while (offset < num);
            filesize -= num;
        } while (filesize > 0);
    }
    return true;
}