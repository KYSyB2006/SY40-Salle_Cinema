#ifndef TICKET_SERVICE_H
#define TICKET_SERVICE_H
#include "struct.h"

int purchase_ticket(Cinema* cinema, int screening_id, const char* name, const char* email, int age, int seat_id);
int exchange_ticket(Cinema* cinema, int ticket_id, int new_screening_id, int new_seat_id);
int cancel_ticket(Cinema* cinema, int ticket_id);
int refund_ticket(Cinema* cinema, int ticket_id);
int verify_age(Movie* movie, int clientAge);

#endif //TICKET_SERVICE_H