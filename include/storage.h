#ifndef STORAGE.H
#define STORAGE .H

typedef struct
{
    int flight_id;
    char origin[50];
    char destination[50];
    char dep_time[20];
    char arr_time[20];
    int total_seats;
    int available_seats;
} Flight;

typedef struct
{
    int booking_id;
    int flight_id;
    char passport[20];
    char country[30];
    char fullname[60];
} Reservation;

#endif