/*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 5550  /* Port that will be opened */ 
#define BUFF_SIZE 1024

const char* handle(char *buff);

int main(int argc, char *argv[])
{
	if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        return 0;
    }

	int port = atoi(argv[1]); /* Port that will be opened */ 

	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client[2]; /* client's address information */
	struct sockaddr_in tmp;
	int sin_size;

	//Step 1: Construct a UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); /* zero the rest of the structure */

	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}

	// Connect to client
	sin_size = sizeof(struct sockaddr_in);
	do
	{
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &tmp, &sin_size);
		
		if (bytes_received < 0)
			perror("\nError: ");
		else{
			buff[bytes_received] = '\0';
			printf("[%s:%d]: %s\n", inet_ntoa(tmp.sin_addr), ntohs(tmp.sin_port), buff);

			if (!client[0].sin_port) {
				client[0] = tmp;
				bytes_sent = sendto(server_sock, "", 0, 0, (struct sockaddr *) &client[0], sin_size );
			} else if (!client[1].sin_port){
				client[1] = tmp;
			}

			if (bytes_sent < 0)
			perror("\nError: ");
		}
	} while (!client[0].sin_port || !client[1].sin_port);
	
	//Step 3: Communicate with clients
	while (1) {
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &tmp, &sin_size);
		if (bytes_received < 0)
			perror("\nError: ");
		else{
			buff[bytes_received] = '\0';
			printf("[%s:%d]: %s\n", inet_ntoa(tmp.sin_addr), ntohs(tmp.sin_port), buff);
		}
	
		char* str = handle(&buff);
		if ((tmp.sin_addr.s_addr == client[0].sin_addr.s_addr) && (tmp.sin_port == client[0].sin_port)) {
			bytes_sent = sendto(server_sock, str, strlen(str), 0, (struct sockaddr *) &client[1], sin_size ); /* send to the client welcome message */
		} else {
			bytes_sent = sendto(server_sock, str, strlen(str), 0, (struct sockaddr *) &client[0], sin_size ); /* send to the client welcome message */
		}
			
		if (bytes_sent < 0)
			perror("\nError: ");
	}
	
	close(server_sock);
	return 0;
}

// handle result
const char* handle(char *buff)
{
	char letters[BUFF_SIZE], digitals[BUFF_SIZE];
	int len = strlen(buff);
	int lenLetters = 0, lenDigitals = 0;

	for (int i = 0; i < len; i++) 
	{
		if ((buff[i] >= 'A' && buff[i] <= 'Z') || (buff[i] >= 'a' && buff[i] <= 'z')) {
			letters[lenLetters++] = buff[i];
		} else if (buff[i] >= '0' && buff[i] <= '9') {
			digitals[lenDigitals++] = buff[i];
		} else {
			return "Error";
		}
	}

	// Return result
	letters[lenLetters] = '\0';
	digitals[lenDigitals] = '\0';
	if (lenDigitals && lenLetters) {
		strcpy(buff, digitals);
		strcat(buff, "\n");
		strcat(buff, letters);
	} else if (lenLetters) {
		strcpy(buff, letters);
	} else if (lenDigitals) {
		strcpy(buff, digitals);
	}
	
	return buff;
}