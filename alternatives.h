#ifndef ALTERNATIVES_H
#define ALTERNATIVES_H
#include "struct.h"

typedef struct
{
   Screening* screening;
   Seat* seat;
}AlternativeChoice;

typedef enum {
    ALT_CHANGE_SEAT,
    ALT_CHANGE_SCREENING,
    ALT_CHANGE_MOVIE
} AlternativeType;

typedef struct {
    AlternativeType type;
    int screening_id;
    int seat_id;
} AlternativeOption;

typedef struct {
    AlternativeOption* options;
    int count;
} AlternativeList;

// liste des places disponibles pour une salle
Seat** list_available_seats(Room* room, int* count);

// liste des screenings disponibles pour un film avec place disponible
Screening** list_screenings_same_movie(Cinema* cinema, Movie* movie, int* count);

// liste des screenings compatibles avec l'age du client
Screening** list_screenings_by_age(Cinema* cinema, int clientAge, int* count);

// liste des screenings associés aux films
Screening** list_screenings(Cinema* cinema, int* count);

//
AlternativeList* compute_alternatives(Cinema* cinema, Ticket* ticket,  const char* reason);

#endif //ALTERNATIVES_H


