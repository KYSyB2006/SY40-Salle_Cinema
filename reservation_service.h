#ifndef RESERVATION_SERVICE_H
#define RESERVATION_SERVICE_H
#include "struct.h"

int make_reservation(Cinema* cinema, int screening_id, const char* name, const char* email, int age, int seat_id);
int modify_reservation(Cinema* cinema, int ticket_id, int new_screening_id, int new_seat_id);
int validate_reservation(Cinema* cinema, int ticket_id);
int cancel_reservation(Cinema* cinema, int ticket_id);
#endif