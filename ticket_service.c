/* Dans ce fichier, on va définir les fonctions pour l'achat,l'échange
    l'annulataion et le remboursement des billets
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include "struct.h"
#include "ticket_service.h"
#include "alternatives.h"

//Mutex pour la gestion des billets
static pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t seat_mutex = PTHREAD_MUTEX_INITIALIZER;

// Achat d'un billet
int purchase_ticket(Cinema* cinema, int screening_id, const char* name, const char* email, int age, int seat_id) {
    
    pthread_mutex_lock(&ticket_mutex);
    pthread_mutex_lock(&seat_mutex);

    //Recupération des entités
    Screening* screening;
    for(int i=0; i<cinema->num_screenings; i++) {
        if(cinema->screenings[i]->id == screening_id) {
            screening = cinema->screenings[i];
            break;
        }
    }

    Room* room = screening->room;

    Seat* seat;
    for(int i=0; i<room->available_seats; i++) {
        if(room->seats[i]->id == seat_id) {
            seat = room->seats[i];
            break;
        }
    }

    int need_alternative = 0;

    //Vérification de la disponibilité du siège
    if(seat->status != SEAT_AVAILABLE) {
        fprintf(stderr, "Achat refusé : siège %d non disponible\n", seat_id);
        need_alternative = 1;
    }

    //Vérification de l'âge
    if(!verify_age(screening->movie, age)) {
        fprintf(stderr, "Achat refusé : restriction d'âge pour le film %s\n", screening->movie->title);
        need_alternative = 1;
    }

    //Gestion des alternatives si besoin
    if(need_alternative) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);

        //Ticket temporaire
        Ticket temp_ticket;
        temp_ticket.age = age;
        temp_ticket.screening = screening;

        //Alternative
        AlternativeChoice* choice = alternatives(&temp_ticket, cinema);
        if(!choice) {
            printf("Achat annulé par l'utilisateur.\n");
            return 0;
        }

        //Poursuite de l'achat avec la nouvelle sélection
        return purchase_ticket(cinema, choice->screening->id, name, email, age, choice->seat->id);

    }

    //Création du billet
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    ticket->id = cinema->num_tickets + 1;
    strncpy(ticket->customer_name, name, 50);
    strncpy(ticket->email, email, 50);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_SOLD;
    ticket->purchase_time = time(NULL);
    ticket->reservation_time = 0;
    ticket->is_reservation = 0;

    //Mise à jour
    seat->status = SEAT_SOLD;
    seat->ticket_id = ticket->id;
    room->available_seats -= 1;
    screening->seats_sold += 1;

    cinema->tickets = (Ticket**)realloc(cinema->tickets, sizeof(Ticket*) * (cinema->num_tickets + 1));
    cinema->tickets[cinema->num_tickets] = ticket;
    cinema->num_tickets += 1;

    pthread_mutex_unlock(&ticket_mutex);
    pthread_mutex_unlock(&seat_mutex);

    return 1;
}

// Echange d'un billet
int exchange_ticket(Cinema* cinema, int ticket_id, int new_screening_id, int new_seat_id) {
    pthread_mutex_lock(&ticket_mutex);
    pthread_mutex_lock(&seat_mutex);

    //Récupération du billet
    Ticket* ticket = NULL;
    for(int i=0; i<cinema->num_tickets; i++) {
        if(cinema->tickets[i]->id == ticket_id) {
            ticket = cinema->tickets[i];
            break;
        }
    }

    if(!ticket) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Echange refusé : billet %d non trouvé\n", ticket_id);
        return 0;
    }

    //Récupération de la nouvelle séance
    Screening* new_screening = NULL;
    for(int i=0; i<cinema->num_screenings; i++) {
        if(cinema->screenings[i]->id == new_screening_id) {
            new_screening = cinema->screenings[i];
            break;
        }
    }

    if(!new_screening) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Echange refusé : séance %d non trouvée\n", new_screening_id);
        return 0;
    }

    Room* new_room = new_screening->room;

    Seat* new_seat = NULL;
    for(int i=0; i<new_room->available_seats; i++) {
        if(new_room->seats[i]->id == new_seat_id) {
            new_seat = new_room->seats[i];
            break;
        }
    }

    int need_alternative = 0;
    //Vérification de la disponibilité du nouveau siège
    if(new_seat->status != SEAT_AVAILABLE) {
        fprintf(stderr, "Echange refusé : siège %d non disponible\n", new_seat_id);
        need_alternative = 1;
    }

    //Vérification de l'âge
    if(!verify_age(new_screening->movie, ticket->age)) {
        fprintf(stderr, "Echange refusé : restriction d'âge pour le film %s\n", new_screening->movie->title);
        need_alternative = 1;
    }

    //Gestion des alternatives si besoin
    if(need_alternative) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);

        //Alternative
        AlternativeChoice* choice = alternatives(ticket, cinema);
        if(!choice) {
            printf("Echange annulé par l'utilisateur.\n");
            return 0;
        }

        //Poursuite de l'échange 
        return exchange_ticket(cinema, ticket_id, choice->screening->id, choice->seat->id);

    }

    //Mise à jour des anciens sièges
    Room* old_room = ticket->screening->room;
    Seat* old_seat = NULL;
    for(int i=0; i<old_room->available_seats; i++) {
        if(old_room->seats[i]->id == ticket->seat_id) {
            old_seat = old_room->seats[i];
            break;
        }
    }

    old_seat->status = SEAT_AVAILABLE;
    old_seat->ticket_id = -1;
    old_room->available_seats += 1;
    ticket->screening->seats_sold -= 1;

    //Mise à jour des nouveaux sièges
    new_seat->status = SEAT_SOLD;
    new_seat->ticket_id = ticket->id;
    new_room->available_seats -= 1;
    new_screening->seats_sold += 1;

    //Mise à jour du billet
    ticket->screening = new_screening;
    ticket->seat_id = new_seat_id;
    ticket->status = TICKET_EXCHANGED;

    pthread_mutex_unlock(&ticket_mutex);
    pthread_mutex_unlock(&seat_mutex);

    return 1;
}
  
// Annulation d'un billet
int cancel_ticket(Cinema* cinema, int ticket_id) {
    pthread_mutex_lock(&ticket_mutex);
    pthread_mutex_lock(&seat_mutex);

    //Récupération du billet
    Ticket* ticket = NULL;
    for(int i=0; i<cinema->num_tickets; i++) {
        if(cinema->tickets[i]->id == ticket_id) {
            ticket = cinema->tickets[i];
            break;
        }
    }

    if(!ticket) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Annulation refusée : billet %d non trouvé\n", ticket_id);
        return 0;
    }

    //Verification du statut du billet
    if(ticket->status != TICKET_CANCELLED && ticket->status != TICKET_REFUNDED){
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Annulation refusée : billet %d n'est pas annulable\n", ticket_id);
        return 0;
    }

    //Contrainte de temps (30min)
    time_t now=time(NULL);
    double diff=difftime(now, ticket->screening->start_time);
    if(diff>1800){
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Annulation refusée : délai d'annulation dépassé %d\n", ticket_id);
        return 0;
    }

    //Recherche du siège
    Room* room = ticket->screening->room;
    Seat* seat = NULL;
    for(int i=0; i<room->available_seats; i++) {
        if(room->seats[i]->id == ticket->seat_id) {
            seat = room->seats[i];
            break;
        }
    }

    //Mise à jour
    seat->status = SEAT_AVAILABLE;
    seat->ticket_id = -1;
    room->available_seats += 1;
    ticket->screening->seats_sold -= 1;

    //Mise à jour du billet
    ticket->status = TICKET_CANCELLED;

    pthread_mutex_unlock(&ticket_mutex);
    pthread_mutex_unlock(&seat_mutex);

    return 1;
}

// Remboursement d'un billet
int refund_ticket(Cinema* cinema, int ticket_id) {
    pthread_mutex_lock(&ticket_mutex);
    pthread_mutex_lock(&seat_mutex);

    //Récupération du billet
    Ticket* ticket = NULL;
    for(int i=0; i<cinema->num_tickets; i++) {
        if(cinema->tickets[i]->id == ticket_id) {
            ticket = cinema->tickets[i];
            break;
        }
    }

    if(!ticket) {
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Remboursement refusé : billet %d non trouvé\n", ticket_id);
        return 0;
    }

    //Verification du statut du billet
    if(ticket->status != TICKET_CANCELLED){
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Remboursement refusé : billet %d n'est pas remboursable\n", ticket_id);
        return 0;
    }

    //Contrainte de temps (3h après l'achat)
    time_t now=time(NULL);
    double diff=difftime(now, ticket->purchase_time);
    if(diff>10800){
        pthread_mutex_unlock(&ticket_mutex);
        pthread_mutex_unlock(&seat_mutex);
        fprintf(stderr, "Remboursement refusé : délai de remboursement dépassé %d\n", ticket_id);
        return 0;
    }

    //Mise à jour du billet
    ticket->status = TICKET_REFUNDED;

    pthread_mutex_unlock(&ticket_mutex);
    pthread_mutex_unlock(&seat_mutex);

    return 1;
}

// Verification de l'âge
int verify_age(Movie* movie, int clientAge) {
    if(!movie) {
        return 0; 
    }

    int age=movie->age_rating;

    switch (age) {
        case AGE_ALL:
            return 1;
        
        case AGE_12:
            if(clientAge >= 12) {
                return 1;
            } else {
                return 0;
            }

        case AGE_16:
            if(clientAge >= 16) {
                return 1;
            } else {
                return 0;
            }

        case AGE_18:
            if(clientAge >= 18) {
                return 1;
            } else {
                return 0;
            }

        default:
            return 0; 
    }
}