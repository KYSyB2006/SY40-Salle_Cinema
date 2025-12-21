#ifndef ALTERNATIVES_H
#define ALTERNATIVES_H
#include "struct.h"

typedef struct
{
   Screening* screening;
   Seat* seat;
}AlternativeChoice;

// liste des places disponibles pour une salle
Seat** list_available_seats(Room* room, int* count);

// liste des screenings disponibles pour un film avec place disponible
Screening** list_screenings_same_movie(Cinema* cinema, Movie* movie, int* count);

// liste des screenings compatibles avec l'age du client
Screening** list_screenings_by_age(Cinema* cinema, int clientAge, int* count);

// liste des screenings associés aux films
Screening** list_screenings(Cinema* cinema, int* count);

// Gestion des alternatives
AlternativeChoice* alternatives(Ticket* ticket, Cinema* cinema);

#endif //ALTERNATIVES_H


