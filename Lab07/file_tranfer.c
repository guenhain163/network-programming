#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "file_tranfer.h"

#define SOCKET_ERROR -1
#define BUFF_SIZE 1024

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

bool writeFile(int sockfd, char *path)
{
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

bool sendFile(char *path, int conn_sock)
{
	FILE *f = fopen(path, "rb");
	if(f == NULL) {
        return false;
    } 

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

	fclose(f);
    return true;
}
