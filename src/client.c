#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //for IP metatroph

#define PORT 8080

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Could not create socket :( \n");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    printf("Trying to connect to Server...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Unable to connect to Server...\n");
        return 1;
    }

    printf("Connected to Server!\n");

    struct
    {
        char origin[50];
        char destination[50];
    } request;

    strcpy(request.origin, "Athens");
    strcpy(request.destination, "London");

    printf("Sending request for search: %s -> %s \n", request.origin, request.destination);
    write(sock, &request, sizeof(request));

    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    printf("Server Response: %s\n", buffer);

    close(sock);
    return 0;
}
