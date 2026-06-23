#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum
{
    MSG_SEARCH_REQ,
    MSG_SEARCH_RES,
    MSG_BOOK_REQ,
    MSG_BOOK_RES
} MsgType;

typedef struct
{
    char origin[50];
    char destination[50];
    char date_start[20];
    char date_end[20];
} SearchRequest;

typedef struct
{
    int flight_id1;
    int flight_id2; // antapokrish
    char passport[20];
    char country[30];
    char fullname[60];
} BookingRequest;

#endif