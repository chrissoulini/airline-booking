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

void flush_input()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void search_flight(int sock)
{
    // Header of Message
    MsgType type = MSG_SEARCH_REQ;
    write(sock, &type, sizeof(MsgType));

    SearchRequest req;

    printf("\n--Flight Search--\n");
    printf("Enter Origin City: ");
    scanf("%49s", req.origin);
    printf("Enter Destination City: ");
    scanf("%49s", req.destination);

    strcpy(req.date_start, "2026-07-01");
    strcpy(req.date_end, "2026-07-10");

    printf("Sending request for search: %s -> %s \n", req.origin, req.destination);
    write(sock, &req, sizeof(SearchRequest));

    MsgType res_type;

    if (read(sock, &res_type, sizeof(MsgType)) > 0)
    {
        if (res_type == MSG_SEARCH_RES)
        {

            SearchResponse response;
            read(sock, &response, sizeof(SearchResponse));

            if (response.is_connecting == -1)
            {
                printf("No flights found for this destination.\n");
            }

            else if (response.is_connecting == 0)
            {
                printf("\n --Direct Flight Found --\n");
                printf("Flight ID: %d\n", response.flight1.flight_id);
                printf("Route: %s -> %s\n", response.flight1.origin, response.flight1.destination);
                printf("Departure: %s | Arrival: %s\n", response.flight1.dep_time, response.flight1.arr_time);
                printf("Available Seats: %d\n", response.flight1.available_seats);
                printf("---------------------\n");
            }
            else if (response.is_connecting == 1)
            {
                printf("\n--Connecting Flight Found(1 Stop)--\n");
                printf("[LEG 1]: ID %d | %s -> %s\n", response.flight1.flight_id, response.flight1.origin, response.flight1.destination);
                printf("         Dep: %s | Arr: %s\n", response.flight1.dep_time, response.flight1.arr_time);
                printf("[LEG 2]: ID %d | %s -> %s\n", response.flight2.flight_id, response.flight2.origin, response.flight2.destination);
                printf("         Dep: %s | Arr: %s\n", response.flight2.dep_time, response.flight2.arr_time);
                printf("----------------------------------------------------\n");
            }
        }
    }
}

void book_flight(int sock)
{
    MsgType type = MSG_BOOK_REQ;
    write(sock, &type, sizeof(MsgType));

    BookingRequest book_req;

    printf("\n--Book a Ticket--\n");
    printf("Enter Flight ID 1 (Direct or First Leg): ");
    scanf("%d", &book_req.flight_id1);

    printf("Enter Flight ID 2 (Enter 0 if Direct flight): ");
    scanf("%d", &book_req.flight_id2);

    printf("Choose Seat Number for Flight 1 (0-49): ");
    scanf("%d", &book_req.seat_number1);

    if (book_req.flight_id2 != 0)
    {
        printf("Choose Seat Number for Flight 2 (0-49): ");
        scanf("%d", &book_req.seat_number2);
    }
    else
    {
        book_req.seat_number2 = -1;
    }

    printf("Enter Passport Number: ");
    scanf("%19s", book_req.passport);

    printf("Enter Country of Issue: ");
    scanf("%29s", book_req.country);

    printf("Enter Full Name: ");
    scanf("%59s", book_req.fullname);

    printf("Sending booking request for Flight ID: %d...\n", book_req.flight_id1);
    write(sock, &book_req, sizeof(BookingRequest));

    MsgType res_type;
    if (read(sock, &res_type, sizeof(MsgType)) > 0)
    {
        if (res_type == MSG_BOOK_RES)
        {
            int success;
            read(sock, &success, sizeof(int));

            if (success == 1)
            {
                printf("Booking Successful! :)\n");
            }
            else
            {
                printf("Booking Failed. No seats or false ID\n");
            }
        }
    }
    return;
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

    int running = 1;
    while (running)
    {

        printf("\n===CHRYMA AIRLINES MENU===\n");
        printf("1. Search Flights\n");
        printf("2. Book a Ticket\n");
        printf("3.Exit\n");
        printf("Choose option (1 or 2 or 3): ");

        int choice;
        scanf("%d", &choice);

        if (choice == 1)
        {
            search_flight(sock);
        }
        else if (choice == 2)
        {
            book_flight(sock);
        }
        else if (choice == 3)
        {
            printf("Exiting...CHRYMA SAYS GOODBYE!\n");
            running = 0;
        }
        else
        {
            printf("Invalid choice.Exiting...\n");
        }

        close(sock);
        return 0;
    }
}
