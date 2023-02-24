#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

/* Thread-specific data */
pthread_key_t key_seed;
int key = 0;

int client_sockets[MAX_CLIENTS];

/* Function to generate a random key between 1 and 25 */
int generate_key()
{
    /* Retrieve seed specific to this thread */
    unsigned int seed = (unsigned int)pthread_getspecific(key_seed);
    /* Use seed to generate key */
    return (rand_r(&seed) % 25) + 1;
}

/* Thread function for handling clients */
void *handle_client(void *args)
{
    int client_socket = *((int *)args);
    char buffer[BUFFER_SIZE];

    while (1)
    {
        /* Receive message from client */
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);

        /* Check for errors */
        if (recv_size == -1)
        {
            perror("recv failed");
            break;
        }

        /* Encrypt/Decrypt the message using Caesar Cipher */
        encrypt_decrypt_caesar(buffer, key);

        /* Send the encrypted/decrypted message back to the client */
        send(client_socket, buffer, recv_size, 0);
    }

    /* Close the socket */
    close(client_socket);
    return NULL;
}

/* Thread function for generating key */
void *key_generation(void *args)
{
    /* Create seed specific to this thread */
    unsigned int seed = (unsigned int)time(0);
    pthread_setspecific(key_seed, (void *)seed);
    while (1)
    {
        key = generate_key();
        printf("Generated key: %d\n", key);

        /* Send key to all connected clients */
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0)
            {
                send(client_sockets[i], &key, sizeof(key), 0);
            }
        }

        /* Sleep for 20 seconds */
        struct timespec ts = {20, 0};
        nanosleep(&ts, NULL);
    }
    return NULL;
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(client_address);
    pthread_t client_threads[MAX_CLIENTS], key_thread;

    /* Create thread-specific data key */
    pthread_key_create(&key_seed, NULL);

    /* Create key generation thread */
    pthread_create(&key_thread, NULL, key_generation, NULL);

    /* Create a socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket failed");
        return 1;
    }

    /* Bind the socket to an IP and port */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        return 1;
    }

    /* Listen for incoming connections */
    listen(server_socket, MAX_CLIENTS);

    int i = 0;
    while (1)
    {
        /* Accept incoming connections */
        client_sockets[i] = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&addrlen);
        if (client_sockets[i] < 0)
        {
            perror("accept failed");
            return 1;
        }

        /* Create a new thread to handle the client */
        pthread_create(&client_threads[i], NULL, handle_client, &client_sockets[i]);

        /* Increment the client counter */
        i++;
    }

    /* Close the server socket */
    close(server_socket);
    return 0;
}