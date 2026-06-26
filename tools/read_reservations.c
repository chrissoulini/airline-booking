#include <stdio.h>
#include </home/chrissy/Desktop/airline-booking/include/storage.h>
#include </home/chrissy/Desktop/airline-booking/include/protocol.h>

int main()
{
    // Χρησιμοποιούμε το πλήρες path για να μην χάνεται ποτέ
    FILE *f = fopen("/home/chrissy/Desktop/airline-booking/data/reservations.dat", "rb");
    if (f == NULL)
    {
        printf("Δεν βρέθηκε το αρχείο στη διαδρομή data/reservations.dat! Ψάχνω εναλλακτικά...\n");
        // Δοκιμή με .data μήπως γράφτηκε έτσι
        f = fopen("/home/chrissy/Desktop/airline-booking/data/reservations.data", "rb");
    }

    if (f == NULL)
    {
        printf("Σφάλμα: Το αρχείο κρατήσεων δεν έχει δημιουργηθεί ακόμα ή είναι άδειο.\n");
        return 1;
    }

    BookingRequest res;
    printf("=== ΛΙΣΤΑ ΑΠΟΘΗΚΕΥΜΕΝΩΝ ΚΡΑΤΗΣΕΩΝ ===\n");
    int count = 1;
    while (fread(&res, sizeof(BookingRequest), 1, f) == 1)
    {
        printf("[%d] Name: %s | Passport: %s | Flight ID 1: %d (Seat: %d) | Flight ID 2: %d (Seat: %d)\n",
               count++, res.fullname, res.passport, res.flight_id1, res.seat_number1, res.flight_id2, res.seat_number2);
    }

    fclose(f);
    return 0;
}