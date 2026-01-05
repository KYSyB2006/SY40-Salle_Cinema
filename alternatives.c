/* Ce fichier va contenir les fonctions alternatives pour l'achat et l'échange d'un billet 
    On va mettre en place un menu qui propose les choix des alternatives et selon le choix
    l'utilsateur pourra faire son choix d'altrenative parmi les propositions et ensuite on va 
    continuer le processus d'achat ou d'échange  du billet
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alternatives.h"

Seat** list_available_seats(Room* room, int* count) {
    *count = 0;
    if(!room) {
        return NULL;
    }

    Seat** seats = malloc(sizeof(Seat*) * room->capacity);
    if(!seats) {
        return NULL;
    }

    for(int i=0; i<room->capacity; i++) {
        if(room->seats[i]->status == SEAT_AVAILABLE) {
            seats[(*count)++] = room->seats[i];
        }
    }

    if(*count == 0) {
        free(seats);
        return NULL;
    }

    return seats;
}

Screening** list_screenings_same_movie(Cinema* cinema, Movie* movie, int* count) {
    *count = 0;
    if(!cinema || !movie) {
        return NULL;
    }

    Screening** screenings = malloc(sizeof(Screening*) * cinema->num_screenings);
    if(!screenings) {
        return NULL;
    }

    for(int i=0; i<cinema->num_screenings; i++) {
        Screening* scr = cinema->screenings[i];
        if(scr->movie->id == movie->id && scr->room->available_seats > 0) {
            screenings[(*count)++] = scr;
        }
    }

    if(*count == 0) {
        free(screenings);
        return NULL;
    }

    return screenings;
}

Screening** list_screenings_by_age(Cinema* cinema, int clientAge, int* count) {
    *count = 0;
    if(!cinema) {
        return NULL;
    }

    Screening** screenings = malloc(sizeof(Screening*) * cinema->num_screenings);
    if(!screenings) {
        return NULL;
    }

    for(int i=0; i<cinema->num_screenings; i++) {
        Screening* scr = cinema->screenings[i];
        int age_rating = scr->movie->age_rating;
        int allowed = 0;

        switch (age_rating) {
            case AGE_ALL:
                allowed = 1;
                break;
            case AGE_12:
                if(clientAge >= 12) allowed = 1;
                break;
            case AGE_16:
                if(clientAge >= 16) allowed = 1;
                break;
            case AGE_18:
                if(clientAge >= 18) allowed = 1;
                break;
        }

        if(allowed && scr->room->available_seats > 0) {
            screenings[(*count)++] = scr;
        }
    }

    if(*count == 0) {
        free(screenings);
        return NULL;
    }

    return screenings;
}

// Gestion des alternatives
AlternativeList* compute_alternatives(Cinema* cinema, Ticket* ticket,  const char* reason) {
    if (!cinema || !ticket) return NULL;

    AlternativeList* list = malloc(sizeof(AlternativeList));
    if (!list) return NULL;

    list->options = NULL;
    list->count = 0;

    int scr_count;
    int seat_count;

    // Changer de place (même séance)
    if(strcmp("seat",reason)==0){
        Seat** seats = list_available_seats(ticket->screening->room, &seat_count);

        for (int i = 0; i < seat_count; i++) {
            list->options = realloc(list->options,
                sizeof(AlternativeOption) * (list->count + 1));

            list->options[list->count++] = (AlternativeOption){
                .type = ALT_CHANGE_SEAT,
                .screening_id = ticket->screening->id,
                .seat_id = seats[i]->id
            };
        }
        free(seats);

        //Changer de séance (même film)
        Screening** scrs = list_screenings_same_movie(
            cinema, ticket->screening->movie, &scr_count);

        for (int i = 0; i < scr_count; i++) {
            int sc;
            Seat** s = list_available_seats(scrs[i]->room, &sc);
            for (int j = 0; j < sc; j++) {
                list->options = realloc(list->options,
                    sizeof(AlternativeOption) * (list->count + 1));

                list->options[list->count++] = (AlternativeOption){
                    .type = ALT_CHANGE_SCREENING,
                    .screening_id = scrs[i]->id,
                    .seat_id = s[j]->id
                };
            }
            free(s);
        }
        free(scrs);
    }
    
    // Changer de film (respect âge)
    if(strcmp("age",reason)==0){
        Screening** age_scrs = list_screenings_by_age(
        cinema, ticket->age, &scr_count);

        for (int i = 0; i < scr_count; i++) {
            int sc;
            Seat** s = list_available_seats(age_scrs[i]->room, &sc);
            for (int j = 0; j < sc; j++) {
                list->options = realloc(list->options,
                    sizeof(AlternativeOption) * (list->count + 1));

                list->options[list->count++] = (AlternativeOption){
                    .type = ALT_CHANGE_MOVIE,
                    .screening_id = age_scrs[i]->id,
                    .seat_id = s[j]->id
                };
            }
            free(s);
        }
        free(age_scrs);
    }
    
    if (list->count == 0) {
        free(list);
        return NULL;
    }

    return list;
}
