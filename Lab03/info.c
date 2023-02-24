#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	struct hostent *h = NULL;
	struct in_addr **addr_list;
	struct in_addr ipv4;
	int i = 0;

	if (argc != 3) {
		printf("ERROR ==>> Usage: ./client -n address\n");
		return -1;
	}

	// Check whether -n or -a is used
	if (strcmp(argv[1], "-n") == 0) {
		h = gethostbyname(argv[argc - 1]);
	} else if (strcmp(argv[1], "-a") == 0) {
		inet_aton(argv[2], &ipv4);
		h = gethostbyaddr(&ipv4, sizeof(ipv4), AF_INET);
	} else {
		printf("ERROR ==>> Valid first parameter: -n or -a\n");
		return -1;
	}

	if (h == NULL) {
		printf("ERROR ==>> gethost..() returned null\n");
		return -1;
	}


	// Print the information received
	printf("==>> Nume: %s\n", h->h_name);

	// Aliases
	printf("==>> Aliasuri: ");
	while (h->h_aliases[i] != NULL) {
		printf("| %s ", h->h_aliases[i]);
		i++;
	}

	// IP addresses
	addr_list = (struct in_addr **)h->h_addr_list;
	printf("\n==>> Adrese IP: ");
	for (i = 1; addr_list[i] != NULL; i++) {
		printf("| %s ", inet_ntoa(*addr_list[i - 1]));
		if (i % 3 == 0) {
			printf("\n");
		}
	}
	printf("\n");

	return 0;
}