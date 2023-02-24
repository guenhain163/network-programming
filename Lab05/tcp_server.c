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
#include "datatypes.h"
#include "llist.h"

#define ACTIVED 1
#define BLOCKED 0
#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024

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

// KHAI BÁO CÁC HÀM
void inputData(); // Nhập data từ file

char *strlwr(char *s);

void clear(); //

int recvData(LIST *l, int conn_sock, int *timesLogin, USER *user, int type);

int sendData(int conn_sock, int data);

void outputData(LIST l);

void reset(USER *user);

int main(int argc, char *argv[])
{
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


    LIST l;
    list_init(&l);
    USER *user = (USER *)malloc(sizeof(USER));
    int timesLogin = 1;

    inputData(&l);
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		while(1) {
            if (strlen(user->username) == 0) {
                send_data = recvData(&l, conn_sock, &timesLogin, user, 0);
            } else {
                send_data = recvData(&l, conn_sock, &timesLogin, user, 1);
            }
            
			//echo to client
            if (!sendData(conn_sock, send_data)) {
                break;
            }

            if (send_data == LOGOUT) {
                bytes_sent = send(conn_sock, user->username, sizeof(user->username), 0); /* send to the client welcome message */
                
                if (bytes_sent <= 0){
                    printf("\nConnection closed");
                    break;
                }

                reset(user);
            } else if (send_data == ERROR_CONNECT) {
                break;
            }

		}//end conversation
		close(conn_sock);	
	}
	
	close(listen_sock);
	return 0;
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

void inputData(LIST *l) // Nhập data từ file
{
    FILE *fin;
    fin = fopen("account.txt", "r");

    if (!fin)
    {
        printf("File Error!");
    }
    else
    {
        while (!feof(fin))
        {
            USER *temp = (USER *)malloc(sizeof(USER));
            fscanf(fin, "%[^ ] %[^ ] %d\n\n", temp->username, temp->password, &temp->status);
            list_push(l, list_node(*temp));
            free(temp);
        }
    }

    fclose(fin);
}

void clear()
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

// 0: user
// 1: pass
int recvData(LIST *l, int conn_sock, int *timesLogin, USER *user, int type) 
{
    int bytes_received;
    char recv_data[BUFF_SIZE];
    NODE *checkUser;

    bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); //blocking
    recv_data[bytes_received] = '\0';
	if (bytes_received <= 0){
		printf("\nConnection closed");
		return ERROR_CONNECT;
	}

    printf("\nReceive: Successfully\n");

    if (strcmp(recv_data, "Bye") == 0 && user->status == 1) {
        return LOGOUT;
    }

    if (!type) {
        checkUser = searchByUsername(l, recv_data);
        if (checkUser == NULL) {
            return USER_NOT_EXIST;
        } else {
            *timesLogin = 1;
            strcpy(user->username, checkUser->data.username);
            return USER_EXIST;
        }
    } else {
        checkUser = searchByUsername(l, user->username);
        if (checkUser->data.status == BLOCKED) {
            reset(user);
            return IS_BLOCKED;
        }

        if (strcmp(recv_data, checkUser->data.password) != 0) {
            if (*timesLogin <= 3) {
                (*timesLogin)++;
                return PASS_INCORRECT;
            } else {
                checkUser->data.status = BLOCKED;
                outputData(*l);
                reset(user);
                return LOGIN_TOO_MUCH;
            }
        }
    }

    strcpy(user->username, recv_data);
    user->status = 1;
    return SUCCESS;
}

int sendData(int conn_sock, int data)
{
    uint32_t buff = htonl(data);
    int bytes_sent;
    bytes_sent = send(conn_sock, &buff, sizeof(buff), 0); /* send to the client welcome message */
	
    if (bytes_sent <= 0){
		printf("\nConnection closed");
		return 0;
	}

    return 1;
}

void outputData(LIST l)
{
    FILE *fin = fopen("account.txt", "wb");

    if (!fin)
    {
        printf("File loi!");
    }
    else
    {
        for (NODE *k = l.pHead; k != NULL; k = k->pNext)
        {
            fprintf(fin, "%s %s %d\n", k->data.username, k->data.password, k->data.status);
        }
    }

    fclose(fin);
}

void reset(USER *user)
{
    strcpy(user->username, "");
    strcpy(user->password, "");
    user->status = 0;
}