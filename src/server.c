#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen;

    // socket creation
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(1);
    }

    // REUSE PORT IF SERVER CLOSED
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    addrlen = sizeof(address);

    // Bind socket port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Server started listening throught the port %d...\n", PORT);

    // Main loop for clients
    while (1)
    {
        printf("Waiting for new client...\n");

        addrlen = sizeof(address);

        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("accept");
            exit(1);
        }

        printf("A client connected...(Socket ID: %d)\n", new_socket);
        // posix thread
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
