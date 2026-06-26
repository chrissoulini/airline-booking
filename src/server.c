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
#define MAX_FLIGHTS 100

pthread_mutex_t reservation_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flight_mutexes[MAX_FLIGHTS];

void init_flight_mutexes()
{
    for (int i = 0; i < MAX_FLIGHTS; i++)
    {
        pthread_mutex_init(&flight_mutexes[i], NULL);
    }
}

void *client_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);

    printf("(Thread) Socket %d handling started...\n", sock);

    MsgType msg_type;

    while (read(sock, &msg_type, sizeof(MsgType)) > 0)
    {
        if (msg_type == MSG_SEARCH_REQ)
        {
            SearchRequest req;
            read(sock, &req, sizeof(SearchRequest));

            printf("(Thread) Searching flights from %s to %s\n", req.origin, req.destination);

            FILE *f = fopen("data/flights.data", "rb");
            if (f == NULL)
            {
                SearchResponse err_res;
                err_res.is_connecting = -1;
                MsgType res_type = MSG_SEARCH_RES;
                write(sock, &res_type, sizeof(MsgType));
                write(sock, &err_res, sizeof(SearchResponse));
                continue;
            }

            Flight all_flights[MAX_FLIGHTS];
            int total_flights = 0;
            while (fread(&all_flights[total_flights], sizeof(Flight), 1, f) == 1)
            {
                total_flights++;
            }
            fclose(f);

            SearchResponse response;
            response.is_connecting = -1;

            for (int i = 0; i < total_flights; i++)
            {
                if (strcmp(all_flights[i].origin, req.origin) == 0 &&
                    strcmp(all_flights[i].destination, req.destination) == 0 &&
                    all_flights[i].available_seats > 0)
                {
                    response.is_connecting = 0;
                    response.flight1 = all_flights[i];
                    break;
                }
            }

            if (response.is_connecting == -1)
            {
                for (int i = 0; i < total_flights; i++)
                {
                    if (strcmp(all_flights[i].origin, req.origin) == 0 && all_flights[i].available_seats > 0)
                    {
                        for (int j = 0; j < total_flights; j++)
                        {
                            if (strcmp(all_flights[i].destination, all_flights[j].origin) == 0 &&
                                strcmp(all_flights[j].destination, req.destination) == 0 &&
                                all_flights[j].available_seats > 0)
                            {
                                if (strcmp(all_flights[j].dep_time, all_flights[i].arr_time) > 0)
                                {
                                    response.is_connecting = 1;
                                    response.flight1 = all_flights[i];
                                    response.flight2 = all_flights[j];
                                    break;
                                }
                            }
                        }
                    }
                    if (response.is_connecting == 1)
                        break;
                }
            }

            MsgType res_type = MSG_SEARCH_RES;
            write(sock, &res_type, sizeof(MsgType));
            write(sock, &response, sizeof(SearchResponse));
        }
        else if (msg_type == MSG_BOOK_REQ)
        {
            BookingRequest book_req;
            read(sock, &book_req, sizeof(BookingRequest));

            printf("(Thread) Booking request for %s (ID1: %d, ID2: %d)\n", book_req.fullname, book_req.flight_id1, book_req.flight_id2);

            int success = 0;
            int id1 = book_req.flight_id1;
            int id2 = book_req.flight_id2;

            if (id1 < id2 || id2 == 0)
            {
                if (id1 < MAX_FLIGHTS)
                    pthread_mutex_lock(&flight_mutexes[id1]);
                if (id2 > 0 && id2 < MAX_FLIGHTS)
                    pthread_mutex_lock(&flight_mutexes[id2]);
            }
            else
            {
                if (id2 > 0 && id2 < MAX_FLIGHTS)
                    pthread_mutex_lock(&flight_mutexes[id2]);
                if (id1 < MAX_FLIGHTS)
                    pthread_mutex_lock(&flight_mutexes[id1]);
            }

            FILE *f = fopen("data/flights.data", "rb+");
            if (f != NULL)
            {
                Flight all_flights[MAX_FLIGHTS];
                int total_flights = 0;
                int idx1 = -1, idx2 = -1;

                while (fread(&all_flights[total_flights], sizeof(Flight), 1, f) == 1)
                {
                    if (all_flights[total_flights].flight_id == id1)
                        idx1 = total_flights;
                    if (id2 > 0 && all_flights[total_flights].flight_id == id2)
                        idx2 = total_flights;
                    total_flights++;
                }

                int cond1 = (idx1 != -1 && book_req.seat_number1 >= 0 && book_req.seat_number1 < MAX_SEATS && all_flights[idx1].seats[book_req.seat_number1] == 0);
                int cond2 = (id2 == 0 || (idx2 != -1 && book_req.seat_number2 >= 0 && book_req.seat_number2 < MAX_SEATS && all_flights[idx2].seats[book_req.seat_number2] == 0));

                if (cond1 && cond2)
                {
                    all_flights[idx1].seats[book_req.seat_number1] = 1;
                    all_flights[idx1].available_seats--;

                    if (id2 > 0)
                    {
                        all_flights[idx2].seats[book_req.seat_number2] = 1;
                        all_flights[idx2].available_seats--;
                    }

                    rewind(f);
                    fwrite(all_flights, sizeof(Flight), total_flights, f);
                    success = 1;

                    pthread_mutex_lock(&reservation_mutex);
                    FILE *rf = fopen("data/reservations.dat", "ab");
                    if (rf != NULL)
                    {
                        fwrite(&book_req, sizeof(BookingRequest), 1, rf);
                        fclose(rf);
                    }
                    pthread_mutex_unlock(&reservation_mutex);
                }
                fclose(f);
            }

            if (id1 < id2 || id2 == 0)
            {
                if (id2 > 0 && id2 < MAX_FLIGHTS)
                    pthread_mutex_unlock(&flight_mutexes[id2]);
                if (id1 < MAX_FLIGHTS)
                    pthread_mutex_unlock(&flight_mutexes[id1]);
            }
            else
            {
                if (id1 < MAX_FLIGHTS)
                    pthread_mutex_unlock(&flight_mutexes[id1]);
                if (id2 > 0 && id2 < MAX_FLIGHTS)
                    pthread_mutex_unlock(&flight_mutexes[id2]);
            }

            MsgType res_type = MSG_BOOK_RES;
            write(sock, &res_type, sizeof(MsgType));
            write(sock, &success, sizeof(int));
        }
    }

    close(sock);
    printf("(Thread) Client disconnected, thread ended.\n");
    return NULL;
}

int main()
{
    init_flight_mutexes();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(1);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    addrlen = sizeof(address);

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

    printf("Multithread Server started listening through port %d...\n", PORT);

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

        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;

        pthread_t sniffer_thread;

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock_ptr) < 0)
        {
            perror("Thread could not be created");
            free(new_sock_ptr);
            continue;
        }

        pthread_detach(sniffer_thread);

        printf("Main: Thread created...Return to accept..\n");
    }

    close(server_fd);
    return 0;
}