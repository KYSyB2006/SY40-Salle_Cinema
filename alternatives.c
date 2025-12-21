/* Ce fichier va contenir les fonctions alternatives pour l'achat et l'échange d'un billet 
    On va mettre en place un menu qui propose les choix des alternatives et selon le choix
    l'utilsateur pourra faire son choix d'altrenative parmi les propositions et ensuite on va 
    continuer le processus d'achat ou d'échange  du billet
*/
#include <stdio.h>
#include <stdlib.h>
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

// Gestion des alternatives*
AlternativeChoice* alternatives(Ticket* ticket, Cinema* cinema) {
    if(!ticket || !cinema){
        return NULL;
    }

    //menu
    int choice;
    printf("\nChoisissez une alternative pour votre billet :\n");
    printf("1. Changer de place (même salle)\n");
    printf("2. Changer de salle (même film)\n");
    printf("3. Changer de film (adapté à votre âge)\n");
    printf("4. Changer de film\n");
    printf("0. Annuler\n");
    printf("Votre choix : ");
    scanf("%d", &choice);

    if(choice == 0) return NULL;

    AlternativeChoice* alt = malloc(sizeof(AlternativeChoice));
    if (!alt) return NULL;

    //changement de place
    if(choice==1){
        int count;
        Seat** seats = list_available_seats(ticket->screening->room, &count);
        if(!seats) {
            printf("Aucune place disponible dans cette salle.\n");
            free(alt);
            return NULL;
        }

        //affichage à l'écran et choix de user
        for(int i=0; i<count; i++) {
            printf("%d. Siège ID: %d (Row: %d, Col: %d)\n", i+1, seats[i]->id, seats[i]->row, seats[i]->col);
        }
        printf("Choisissez une place : ");
        scanf("%d", &choice);

        //validation du choix et affectation
        if(choice < 1 || choice > count) {
            printf("Choix invalide.\n");
            free(seats);
            free(alt);
            return NULL;
        }

        alt->screening = ticket->screening;
        alt->seat = seats[choice - 1];
        free(seats);
        return alt;
    }

    //changement de salle
    if(choice==2){
        int count;
        Screening** screenings = list_screenings_same_movie(cinema, ticket->screening->movie, &count);
        if(!screenings) {
            printf("Aucune autre salle disponible pour ce film.\n");
            free(alt);
            return NULL;
        }

        //affichage à l'écran et choix de user
        for(int i=0; i<count; i++) {
            printf("%d. Séance ID: %d (Salle: %s, Heure: %s)\n", i+1, screenings[i]->id, screenings[i]->room->name, ctime(&screenings[i]->start_time));
        }
        printf("Choisissez une séance : ");
        scanf("%d", &choice);

        //validation du choix et affectation
        if(choice < 1 || choice > count) {
            printf("Choix invalide.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //liste des places disponibles dans la nouvelle salle
        int seat_count;
        Seat** seats = list_available_seats(screenings[choice - 1]->room, &seat_count);
        if(!seats) {
            printf("Aucune place disponible dans cette salle.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //affichage des places disponibles
        for(int i=0; i<seat_count; i++) {
            printf("%d. Siège ID: %d (Row: %d, Col: %d)\n", i+1, seats[i]->id, seats[i]->row, seats[i]->col);
        }
        printf("Choisissez une place : ");
        int seat_choice;
        scanf("%d", &seat_choice);

        //validation du choix de siège et affectation
        if(seat_choice < 1 || seat_choice > seat_count) {
            printf("Choix invalide.\n");
            free(seats);
            free(screenings);
            free(alt);
            return NULL;
        }

        alt->screening = screenings[choice - 1];
        alt->seat = seats[seat_choice - 1];

        free(seats);
        free(screenings);
        return alt;
    }

    //changer de film avec contrainte d'age
    if(choice==3){
        int count;
        Screening** screenings = list_screenings_by_age(cinema, ticket->age, &count);
        if(!screenings) {
            printf("Aucun film disponible adapté à votre âge.\n");
            free(alt);
            return NULL;
        }

        //affichage des films et screenings et choix de user
        //choice nous donne implicitement l'index du screening associé au film choisi
        for(int i=0; i<count; i++) {
            printf("%d. Film: %s, Séance ID: %d (Salle: %s, Heure: %s)\n", i+1, screenings[i]->movie->title, screenings[i]->id, screenings[i]->room->name, ctime(&screenings[i]->start_time));
        }
        printf("Choisissez un film : ");
        scanf("%d", &choice);

        //validation du choix et affectation
        if(choice < 1 || choice > count) {
            printf("Choix invalide.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //Recherche des places
        int seat_count;
        Seat** seats = list_available_seats(screenings[choice - 1]->room, &seat_count);
        if(!seats) {
            printf("Aucune place disponible dans cette salle.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //affichage des places disponibles et choix
        for(int i=0; i<seat_count; i++) {
            printf("%d. Siège ID: %d (Row: %d, Col: %d)\n", i+1, seats[i]->id, seats[i]->row, seats[i]->col);
        }
        printf("Choisissez une place : ");
        int seat_choice;
        scanf("%d", &seat_choice);

        //validation du choix de siège et affectation
        if(seat_choice < 1 || seat_choice > seat_count) {
            printf("Choix invalide.\n");
            free(seats);
            free(screenings);
            free(alt);
            return NULL;
        }

        alt->screening = screenings[choice - 1];
        alt->seat = seats[seat_choice - 1];

        free(seats);
        free(screenings);
        return alt;
    }

    //changer de film (c'est le mm contenu que le choix 3 juste que on ne filtre pas les screenings)
    if(choice==4){
        int count;
        Screening** screenings = list_screenings(cinema, &count);
        if(!screenings) {
            printf("Aucun film disponible.\n");
            free(alt);
            return NULL;
        }

        //affichage des films et screenings et choix de user
        for(int i=0; i<count; i++) {
            printf("%d. Film: %s, Séance ID: %d (Salle: %s, Heure: %s)\n", i+1, screenings[i]->movie->title, screenings[i]->id, screenings[i]->room->name, ctime(&screenings[i]->start_time));
        }
        printf("Choisissez un film : ");
        scanf("%d", &choice);

        //validation du choix et affectation
        if(choice < 1 || choice > count) {
            printf("Choix invalide.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //Recherche des places
        int seat_count;
        Seat** seats = list_available_seats(screenings[choice - 1]->room, &seat_count);
        if(!seats) {
            printf("Aucune place disponible dans cette salle.\n");
            free(screenings);
            free(alt);
            return NULL;
        }

        //affichage des places disponibles et choix
        for(int i=0; i<seat_count; i++) {
            printf("%d. Siège ID: %d (Row: %d, Col: %d)\n", i+1, seats[i]->id, seats[i]->row, seats[i]->col);
        }
        printf("Choisissez une place : ");
        int seat_choice;
        scanf("%d", &seat_choice);

        //validation du choix de siège et affectation
        if(seat_choice < 1 || seat_choice > seat_count) {
            printf("Choix invalide.\n");
            free(seats);
            free(screenings);
            free(alt);
            return NULL;
        }

        alt->screening = screenings[choice - 1];
        alt->seat = seats[seat_choice - 1];

        free(seats);
        free(screenings);
        return alt;
    }

    free(alt);
    return NULL;
}