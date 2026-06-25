#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include </home/chrissy/Desktop/airline-booking/include/storage.h>
#include </home/chrissy/Desktop/airline-booking/include/protocol.h>

#define PORT 8080

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);

    printf("(Thread)Socket %d handling started...\n", sock);

    MsgType msg_type;

    if (read(sock, &msg_type, sizeof(MsgType)) <= 0)
    {
        close(sock);
        return NULL;
    }

    if (msg_type == MSG_SEARCH_REQ)
    {
        SearchRequest req;
        read(sock, &req, sizeof(SearchRequest));

        printf("(Thread) Searching flights from %s to %s\n", req.origin, req.destination);

        printf("[DEBUG 1]\n");
        pthread_mutex_lock(&file_mutex);
        printf("[DEBUG 2]\n");

        FILE *f = fopen("data/flights.data", "rb");
        if (f == NULL)
        {
            printf("[DEBUG ERROR 3]\n");
            pthread_mutex_unlock(&file_mutex);
            close(sock);
            return NULL;
        }

        Flight current_flight;
        int found = 0;

        while (fread(&current_flight, sizeof(Flight), 1, f) == 1)
        {
            if (strcmp(current_flight.origin, req.origin) == 0 && strcmp(current_flight.destination, req.destination) == 0)
            {
                MsgType res_type = MSG_SEARCH_RES;
                write(sock, &res_type, sizeof(MsgType));

                write(sock, &current_flight, sizeof(Flight));
                found = 1;
                break;
            }
        }

        fclose(f);
        pthread_mutex_unlock(&file_mutex);

        if (!found)
        {
            MsgType res_type = MSG_SEARCH_RES;
            write(sock, &res_type, sizeof(MsgType));

            Flight empty_flight;
            empty_flight.flight_id = -1;
            write(sock, &empty_flight, sizeof(Flight));
        }
    }

    else if (msg_type == MSG_BOOK_REQ)
    {
        BookingRequest book_req;
        read(sock, &book_req, sizeof(BookingRequest));

        printf("(Thread) Booking request for %s (Flight ID: %d)\n", book_req.fullname, book_req.flight_id1);
        int success = 0;

        pthread_mutex_lock(&file_mutex);
        FILE *f = fopen("data/flights.dat", "rb+");
        if (f != NULL)
        {
            Flight current_flight;
            while (fread(&current_flight, sizeof(Flight), 1, f) == 1)
            {
                if (current_flight.flight_id == book_req.flight_id1)
                {
                    if (current_flight.available_seats > 0)
                    {
                        current_flight.available_seats--;

                        fseek(f, -sizeof(Flight), SEEK_CUR);
                        fwrite(&current_flight, sizeof(Flight), 1, f);
                        success = 1;
                    }
                    break;
                }
            }
            fclose(f);
        }
        pthread_mutex_unlock(&file_mutex);

        MsgType res_type = MSG_BOOK_RES;
        write(sock, &res_type, sizeof(MsgType));
        write(sock, &success, sizeof(int));
    }

    close(sock);
    printf("(Thread) Thread ended\n");
    return NULL;
}

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

    printf("Multithread Server started listening throught the port %d...\n", PORT);

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
        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;

        pthread_t sniffer_thread; // metablhth gia ID Thread

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock_ptr) < 0)
        {
            perror("Thread could not be created");
            free(new_sock_ptr);
            continue;
        }

        pthread_detach(sniffer_thread); // erase the thread from memory gia na mhn exoyme zombie

        printf("Main: Thread created...Return to accept..\n");
    }
    close(server_fd);
    return 0;
}
