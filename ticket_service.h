#ifndef TICKET_SERVICE_H
#define TICKET_SERVICE_H
#include "struct.h"
#include "alternatives.h"

typedef enum {
    OK = 1,
    INVALID =0 ,
    SEAT_UNAVAILABLE=10,
    AGE_DENIED=11
} TicketResult;

TicketResult purchase_ticket(Cinema* cinema, int screening_id, const char* name, const char* email, int age, int seat_id, AlternativeList** alt);
TicketResult exchange_ticket(Cinema* cinema, int ticket_id, int new_screening_id, int new_seat_id, AlternativeList** alt);
int cancel_ticket(Cinema* cinema, int ticket_id);
int refund_ticket(Cinema* cinema, int ticket_id);
int verify_age(Movie* movie, int clientAge);

#endif //TICKET_SERVICE_H