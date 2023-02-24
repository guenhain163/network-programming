#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <stdbool.h>

int DNSLookUp(char* hostname, char* ip);

bool isValidIpAddress(char *ipAddress);


int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s hostname\n", argv[0]);
        return 1;
    }
    
    char hostname[100];
    char ip[100];
    char *tmp = argv[1];
    // Check argv[1] is ip?
    if(isValidIpAddress(tmp)) {
        strcpy(ip, tmp);
    } else {
        strcpy(hostname, tmp);
    }
    
    DNSLookUp(hostname, ip);

    return 0;
}


int DNSLookUp(char* hostname, char* ip) 
{
    struct hostent *H;
    struct in_addr **AddrList;

    // Check hostname is Null?
    if (strlen(hostname) > 0) {
        if ((H = gethostbyname(hostname)) != NULL)
        {
            AddrList = (struct in_addr **) H->h_addr_list;

            for (int i = 0; AddrList[i] != NULL; i++)
            {
                if (i == 0)
                {
                    printf("Official IP: %s \n", inet_ntoa(*AddrList[i]));
                    printf("Alias IP:\n");
                } else {
                    printf("%s\n", inet_ntoa(*AddrList[i]));
                }
            }

            return 1;
        }
    }

    // Check ip is Null?
    if (strlen(ip) > 0) {
        struct in_addr addr;
        inet_aton(ip, &addr);

        if ((H = gethostbyaddr(&addr, sizeof(addr), AF_INET)) != NULL)
        {
            printf("Official name: %s\n", H->h_name);
            strcpy(hostname, H->h_name);
        }

        printf("Alias name:\n");
        for (int i = 0; H->h_aliases[i] != NULL; i++)
        {
            printf("%s", H->h_aliases[i]);
        }
        return 1;
    }

    printf("No Information found\n");
    return 1;
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}