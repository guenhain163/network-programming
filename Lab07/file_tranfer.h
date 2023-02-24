#ifndef __FILE_TRANFER__
#define __FILE_TRANFER__

bool readFile(int sock, FILE *f);


bool writeFile(int sockfd, char *filename);

bool readData(int sock, void *buf, int buflen);

bool readLong(int sock, long *value);

bool sendLong(int sock, long value);

bool sendData(int sock, void *buf, int buflen);

bool sendFile(char *path, int conn_sock);

#endif