#ifndef STORAGE_H
#define STORAGE_H

#define MAX_SEATS 50

typedef struct
{
    int flight_id;
    char origin[50];
    char destination[50];
    char dep_time[20];
    char arr_time[20];
    int seats[MAX_SEATS];
    int available_seats;
} Flight;

typedef struct
{
    int booking_id;
    int flight_id;
    int seat_number;
    char passport[20];
    char country[30];
    char fullname[60];
} Reservation;

#endif