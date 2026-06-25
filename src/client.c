#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //for IP metatroph
#include </home/chrissy/Desktop/airline-booking/include/storage.h>
#include </home/chrissy/Desktop/airline-booking/include/protocol.h>

#define PORT 8080

void search_flight(int sock)
{
    // Header of Message
    MsgType type = MSG_SEARCH_REQ;
    write(sock, &type, sizeof(MsgType));

    SearchRequest req;
    strcpy(req.origin, "Athens");
    strcpy(req.destination, "London");
    strcpy(req.date_start, "2026-07-01");
    strcpy(req.date_end, "2026-07-10");

    printf("Sending request for seach: %s -> %s \n", req.origin, req.destination);

    write(sock, &req, sizeof(SearchRequest));

    MsgType res_type;
    if (read(sock, &res_type, sizeof(MsgType)) > 0)
    {
        if (res_type == MSG_SEARCH_RES)
        {
            Flight res_flight;

            read(sock, &res_flight, sizeof(Flight));

            if (res_flight.flight_id == -1)
            {
                printf("No flights for this destination.\n");
            }
            else
            {
                printf("\n--FLIGHT FOUND--\n");
                printf("Flight ID: %d\n", res_flight.flight_id);
                printf("Route: %s -> %s\n", res_flight.origin, res_flight.destination);
                printf("Departure: %s | Arrival: %s\n", res_flight.dep_time, res_flight.arr_time);
                printf("Available Seats: %d\n", res_flight.available_seats);
                printf("---------------------\n");
            }
        }
    }
    close(sock);
    return 0;
}

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

    printf("Connected to CHRYMA Server!\n");

    printf("\n===CHRYMA AIRLINES MENU===\n");
    printf("1. Search Flights\n");
    printf("2. Book a Ticket\n");
    printf("")
}
