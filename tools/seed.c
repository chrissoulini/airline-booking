#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/home/chrissy/Desktop/airline-booking/include/storage.h"

int main()
{

    FILE *f = fopen("data/flights.data", "wb");
    if (!f)
    {
        perror("Unable to create flight file");
        return 1;
    }

    Flight table[3];

    table[0].flight_id = 1;
    strcpy(table[0].origin, "Athens");
    strcpy(table[0].destination, "Thessaloniki");
    strcpy(table[0].dep_time, "2026-07-01 10:00");
    strcpy(table[0].arr_time, "2026-07-01 11:00");
    table[0].available_seats = MAX_SEATS;
    memset(table[0].seats, 0, sizeof(table[0].seats));

    table[1].flight_id = 2;
    strcpy(table[1].origin, "Thessaloniki");
    strcpy(table[1].destination, "London");
    strcpy(table[1].dep_time, "2026-07-01 14:00");
    strcpy(table[1].arr_time, "2026-07-01 17:00");
    table[1].available_seats = MAX_SEATS;
    memset(table[1].seats, 0, sizeof(table[1].seats));

    // Πτήση 3 (για να υπάρχει εναλλακτική ανταπόκριση)
    table[2].flight_id = 3;
    strcpy(table[2].origin, "Athens");
    strcpy(table[2].destination, "London");
    strcpy(table[2].dep_time, "2026-07-01 08:00");
    strcpy(table[2].arr_time, "2026-07-01 12:00");
    table[2].available_seats = 0;

        for (int i = 0; i < MAX_SEATS; i++)
    {
        table[2].seats[i] = 1;
    }

    fwrite(table, sizeof(Flight), 3, f);
    fclose(f);

    printf("Flight File created succesfully!\n");
    return 0;
}