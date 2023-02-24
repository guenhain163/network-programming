#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "datatypes.h"
#include "llist.h"

#define SOCKET_ERROR -1
#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024

// KHAI BÁO CÁC HÀM
void inputData(); // Nhập data từ file

void clear(); //

bool recvFilename(int conn_sock, char *filename);

bool sendLong(int sock, long value);

bool sendData(int sock, void *buf, int buflen);

bool sendFile(FILE *f, int conn_sock);

bool sendMessage(int conn_sock, char *message);

int main(int argc, char *argv[])
{
    LIST l;
    char filename[BUFF_SIZE];
    char message[BUFF_SIZE];
    list_init(&l);
    inputData(&l);

	if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 0;
    }

	int port = atoi(argv[1]); /* Port that will be opened */ 

	int listen_sock, conn_sock; /* file descriptors */
	int send_data = 0;
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 4: Communicate with client
	while (true) {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
            perror("\nError: ");
        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // start conversation
        
        // Receive filename
        recvFilename(conn_sock, &filename);

        // Check file is exist
        NODE *node = searchByFilename(&l, filename);
        FILE *fp;
        if (node == NULL) {
            strcpy(message, "Error: File Not Found!");
        } else {
            fp  = fopen(node->data.path, "rb");
            if(fp == NULL) {
                perror("[-] Error in reading file.\n");
                exit( 0);
            } else strcpy(message, "Transferring");
        }
            
        message[strlen(message)] = '\0';
        if (!sendMessage(conn_sock, message)) {
            printf("Connect close!\n");
            exit( 0);
        }
            
        if (strcmp(message, "Transferring\0") == 0 && fp) {
            sendFile(fp, conn_sock);
            fclose(fp);
        }
    }

    deleteList(&l);
	close(conn_sock);
	close(listen_sock);
	return 0;
}

void inputData(LIST *l) // Nhập data từ file
{
    FILE *fin;
    fin = fopen("list.txt", "r");

    if (!fin) {
        printf("File Error!");
    } else {
        while (!feof(fin)) {
            IMAGE *temp = (IMAGE *)malloc(sizeof(IMAGE));
            fscanf(fin, "%[^ ] %[^\n]\n", temp->filename, temp->path);
            list_push(l, list_node(*temp));
            free(temp);
        }
    }

    fclose(fin);
}

bool recvFilename(int conn_sock, char *result)
{
    char filename[BUFF_SIZE];
	int bytes_received = recv(conn_sock, filename, BUFF_SIZE-1, 0); //blocking
	if (bytes_received <= 0) {
        return false;
	}
	filename[bytes_received] = '\0';
	printf("\nReceive: %s\n", filename);
    strcpy(result, filename);
    return true;
}

bool sendData(int sock, void *buf, int buflen)
{
    unsigned char *pbuf = (unsigned char *) buf;

    while (buflen > 0)
    {
        int num = send(sock, pbuf, buflen, 0);
        
        if (num == SOCKET_ERROR)
        {
            return false;
        }

        pbuf += num;
        buflen -= num;
    }

    return true;
}

bool sendLong(int sock, long value)
{
    value = htonl(value);
    return sendData(sock, &value, sizeof(value));
}

bool sendFile(FILE *f, int conn_sock)
{
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    if (filesize == EOF)
        return false;
    if (!sendLong(conn_sock, filesize))
        return false;
    if (filesize > 0)
    {
        char buffer[BUFF_SIZE];
        do
        {
            size_t num = filesize < sizeof(buffer) ? filesize : sizeof(buffer);
            num = fread(buffer, 1, num, f);

            if (!sendData(conn_sock, buffer, num))
                return false;
            filesize -= num;

        } while (filesize > 0);
    }

    return true;
}

bool sendMessage(int conn_sock, char *message)
{
    int bytes_sent = send(conn_sock, message, strlen(message), 0);
	if(bytes_sent <= 0){
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
