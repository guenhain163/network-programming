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
#include "file_tranfer.h"

#define PORT 5550
#define BACKLOG 20
#define BUFF_SIZE 1024

/* Handler process signal*/
void sig_chld(int signo);

void toUpper(char *str);

void convertFile(char *path, char *filename);


int main(int argc, char *argv[]) {
	if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 0;
    }

	int port;
	port = atoi(argv[1]);\

	int listen_sock, conn_sock; /* file descriptors */
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	pid_t pid;
	int sin_size;
	char filename[BUFF_SIZE];

	if ((listen_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		printf("socket() error\n");
		return 0;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\nError: ");
		return 0;
	}     

	if(listen(listen_sock, BACKLOG) == -1){  
		perror("\nError: ");
		return 0;
	}
	
	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		
		conn_sock = accept(listen_sock, (struct sockaddr *)&client, (socklen_t*) &sin_size);

		if (conn_sock == -1) {
			if (errno == EINTR)
				continue;
			else {
				perror("\nError: ");			
				return 0;
			}
		}
		
		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();
		
		/* fork() is called in child process */
		if(pid == 0){
			close(listen_sock);
			printf("\nYou got a connection from %c\n", *inet_ntoa(client.sin_addr)); /* prints client's IP */
			
			if (!readData(conn_sock, filename, BUFF_SIZE)) {
				printf("\nError reading filename!\n");
			} else {
				char path[BUFF_SIZE] = {0};
				strcat(path, "Fserver/Origin/");
				strcat(path, filename);

				if (!writeFile(conn_sock, path)) {
					printf("\nError reading file!\n");
				} else 
					convertFile(path, filename);
				
				if (!sendData(conn_sock, filename, sizeof(filename))) {
					printf("\nError sending filename!\n");
				}
				
				if (!sendFile(path, conn_sock)) {
					printf("\nError sending file!\n");
				}

				char buff[BUFF_SIZE];
				strcpy(buff, "Successfully!");
				if (!sendData(conn_sock, buff, sizeof(buff))) {
					printf("\nError sending message!\n");
				} else {
					printf("\n%s\n", buff);
				}
			}

			exit(0);
		}
		
		/* The parent closes the connected socket since the child handles the new client */
		close(conn_sock);
	}
	close(listen_sock);
	return 0;
}

void sig_chld(int signo){
	pid_t pid;
	int stat;
	
	/* Wait the child process terminate */
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("\nChild %d terminated\n", pid);
}

void toUpper(char *str)
{
	int i = 0;
	while(str[i] != '\0') {
		str[i] = toupper(str[i]);
		i++;
	}
}

void convertFile(char *path, char *filename)
{
	FILE *origin = fopen(path, "rb");
	if (origin == NULL) {
        printf("\nError opening file!\n");
    }
	
	strcpy(path, "Fserver/New/");
	toUpper(filename);
	strcat(path, filename);

	FILE *new = fopen(path, "wb");
	if (new == NULL) {
        printf("\nError opening file!\n");
    }
	
	char ch;
	while((ch = fgetc(origin)) != EOF)	{
        ch = toupper(ch);
        fputc(ch, new);
    }
	
	fclose(origin);
	fclose(new);
}