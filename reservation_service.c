/* Dans ce fichier, on va définir les fonctions pour la création, la modification,
   la validation et l'annulation des réservations
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
#include "reservation_service.h"

static pthread_mutex_t seat_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;

// Création d'une réservation
/* Une réservation est presque similaire à un achat de billet, sauf que on ne la considère pas comme telle, 
    elle réprésente une intenttion d'achat stockée sous forme de ticket temporaire.
     Elle devient donc un billet lorsque on fait sa validation
*/
int make_reservation(Cinema* cinema, int screening_id, const char* name, const char* email, int age, int seat_id) {
    pthread_mutex_lock(&seat_mutex);

    //Recuperation des entités
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

    //Verification de l'age
    if(!verify_age(screening->movie, age)) {
        fprintf(stderr, "Réservation refusée : restriction d'âge pour le film %s\n", screening->movie->title);
        return 0;
    }

    //Création de la réservation
    Ticket* reservation = (Ticket*)malloc(sizeof(Ticket));
    reservation->id = rand(); // Pour ne pas empoisonner avec les billets reels
    strncpy(reservation->customer_name, name, 50);
    strncpy(reservation->email, email, 50);
    reservation->age = age;
    reservation->screening = screening;
    reservation->seat_id = seat_id;
    reservation->status = TICKET_VALID;
    reservation->purchase_time = 0;
    reservation->reservation_time = time(NULL);
    reservation->is_reservation = 1;

    //Mise à jour
    seat->status = SEAT_RESERVED;
    seat->ticket_id = reservation->id;
    room->available_seats -= 1;
    screening->seats_reserved += 1;

    //Ajout à la liste des tickets de hotesse
    pthread_mutex_lock(&cinema->reservation_list->mutex);

    //Création du noeud
    TicketNode* rnode = (TicketNode*)malloc(sizeof(TicketNode));
    rnode->ticket = reservation;
    rnode->next = NULL;
    
    // Ajout à la liste
    if(cinema->reservation_list->head == NULL) {
        cinema->reservation_list->head = rnode;
    } else {
        TicketNode* current = cinema->reservation_list->head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = rnode;
    }
    cinema->reservation_list->size += 1;

    pthread_mutex_unlock(&cinema->reservation_list->mutex);
    pthread_mutex_unlock(&seat_mutex);

    return 1;
}

// Modification d'une réservation
int modify_reservation(Cinema* cinema, int ticket_id, int new_screening_id, int new_seat_id) {
    pthread_mutex_lock(&cinema->reservation_list->mutex);

    //Recherche et récupération de la réservation
    TicketNode* current = cinema->reservation_list->head;
    while (current != NULL)
    {
        Ticket* t = current->ticket;
        if(t->id == ticket_id && t->is_reservation && t->status == TICKET_VALID) {
           pthread_mutex_unlock(&cinema->reservation_list->mutex); // pour l'accès pour d'autres op par les autres hotesses
           pthread_mutex_lock(&seat_mutex);

           //off ancien siège et update
            Room* old_room = t->screening->room;
            Seat* old_seat = NULL;
            for(int i=0; i<old_room->available_seats; i++) {
                if(old_room->seats[i]->id == t->seat_id) {
                    old_seat = old_room->seats[i];
                    break;  
                }
            }
            old_seat->status = SEAT_AVAILABLE;
            old_seat->ticket_id = -1;
            old_room->available_seats += 1;
            t->screening->seats_reserved -= 1;

            //on récupère la nouvelle séance
            Screening* new_screening = NULL;
            for(int i=0; i<cinema->num_screenings; i++) {
                if(cinema->screenings[i]->id == new_screening_id) {
                    new_screening = cinema->screenings[i];
                    break;
                }
            }
            if(!new_screening) {
                pthread_mutex_unlock(&seat_mutex);
                fprintf(stderr, "Modification refusée : séance %d non trouvée\n", new_screening_id);
                return 0;
            }
            // le nouveau siege
            Room* new_room = new_screening->room;
            Seat* new_seat = NULL;
            for(int i=0; i<new_room->available_seats; i++) {
                if(new_room->seats[i]->id == new_seat_id) {
                    new_seat = new_room->seats[i];
                    break;
                }
            }
            //check age
            if(!verify_age(new_screening->movie, t->age)) {
                pthread_mutex_unlock(&seat_mutex);
                fprintf(stderr, "Modification refusée : restriction d'âge pour le film %s\n", new_screening->movie->title);
                return 0;
            } 
            //update
            new_seat->status = SEAT_RESERVED;
            new_seat->ticket_id = t->id;
            new_room->available_seats -= 1;
            new_screening->seats_reserved += 1;

            t->screening = new_screening;
            t->seat_id = new_seat_id;
            t->reservation_time = time(NULL);

            pthread_mutex_unlock(&seat_mutex);
            return 1;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&cinema->reservation_list->mutex);
    fprintf(stderr, "Modification refusée : réservation %d non trouvée\n", ticket_id);
    return 0;   
}

// Validation d'une réservation (elle devient un billet)
int validate_reservation(Cinema* cinema, int ticket_id) {
    pthread_mutex_lock(&cinema->reservation_list->mutex);

    //retirer la reservation de la liste
    TicketNode* current = cinema->reservation_list->head;
    TicketNode* prev = NULL;
    Ticket* r = NULL;
    while (current != NULL)
    {
        Ticket* t = current->ticket;
        if(t->id == ticket_id && t->is_reservation && t->status == TICKET_VALID) {
            //on la retire de la liste
            if(prev == NULL) {
                cinema->reservation_list->head = current->next;
            } else {
                prev->next = current->next;
            }
            cinema->reservation_list->size -= 1;
            r = t;
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&cinema->reservation_list->mutex);

    if(!r) {
        fprintf(stderr, "Validation refusée : réservation %d non trouvée\n", ticket_id);
        return 0;
    }

    pthread_mutex_lock(&seat_mutex);
    pthread_mutex_lock(&ticket_mutex);
    //update des champs
    Seat* seat = NULL;
    Room* room = r->screening->room;
    for(int i=0; i<room->available_seats; i++) {
        if(room->seats[i]->id == r->seat_id) {
            seat = room->seats[i];
            break;
        }
    }
    seat->status = SEAT_SOLD;
    r->is_reservation = 0;
    r->purchase_time = time(NULL);
    r->status = TICKET_SOLD;
    r->id=cinema->num_tickets + 1; //nouvel id de billet
    seat->ticket_id = r->id;
    r->screening->seats_sold += 1;
    r->screening->seats_reserved -= 1;
    cinema->tickets = (Ticket**)realloc(cinema->tickets, sizeof(Ticket*) * (cinema->num_tickets + 1));
    cinema->tickets[cinema->num_tickets] = r;
    cinema->num_tickets += 1;

    pthread_mutex_unlock(&ticket_mutex);
    pthread_mutex_unlock(&seat_mutex);
    return 1;
}

// Annulation d'une réservation
int cancel_reservation(Cinema* cinema, int ticket_id){
    pthread_mutex_lock(&cinema->reservation_list->mutex);

    //retirer la reservation de la liste
    TicketNode* current = cinema->reservation_list->head;
    TicketNode* prev = NULL;
    Ticket* r = NULL;
    while (current != NULL)
    {
        Ticket* t = current->ticket;
        if(t->id == ticket_id && t->is_reservation && t->status == TICKET_VALID) {
            //on la retire de la liste
            if(prev == NULL) {
                cinema->reservation_list->head = current->next;
            } else {
                prev->next = current->next;
            }
            cinema->reservation_list->size -= 1;
            r = t;
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&cinema->reservation_list->mutex);

    if(!r) {
        fprintf(stderr, "Annulation refusée : réservation %d non trouvée\n", ticket_id);
        return 0;
    }

    pthread_mutex_lock(&seat_mutex);
    //update
    Seat* seat = NULL;
    Room* room = r->screening->room;
    for(int i=0; i<room->available_seats; i++) {
        if(room->seats[i]->id == r->seat_id) {
            seat = room->seats[i];
            break;
        }
    }
    
    seat->status = SEAT_AVAILABLE;
    seat->ticket_id = -1; 
    r->is_reservation = 0;
    r->status = TICKET_CANCELLED; 
    r->screening->seats_reserved -= 1;
    room->available_seats += 1;
    pthread_mutex_unlock(&seat_mutex);
    
    return 1;
}