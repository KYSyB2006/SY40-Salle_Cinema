#include "structure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

//Constructeur de cinema
Cinema* cinema_create(int num_rooms_total);
//Destructeur de cinema
//void cinema_destroy(Cinema* cinema);

//Constructeur de salle
Room* room_create(const char* name, int rows, int cols);
//Destructeur de salle
//void room_destroy(Room* room);

//Constructeur de film
Movie* movie_create(int id, const char* title, int duration, AgeRating rating, char* genre);
//Destructeur de film
//void movie_destroy(Movie* movie);

//Constructeur de visionnage
Screening* screening_create(Movie* movie, Room* room, time_t start_time, float price);
//Destructeur de visionnage
//void screening_destroy(Screening* screening);

//Constructeur de ticket
//Ticket* ticket_create(char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t time, int is_reservation);
//Destructeur de siege
//void ticket_destroy(Screening* screening);

//fonction pour l'achat des billets
Ticket* achat_ticket(Cinema* cinema, char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t buyingtime);

//fonction pour la reservation des billets
Ticket* reservation_ticket(Cinema* cinema, char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t reservationtime);

//fonction pour la gestion de l'annulation d'une reservation
int annulation_ticket(Cinema* cinema, Ticket* ticket, TicketStatus status);

//fonction permettant l'echange de billet
Ticket* echange_billet(Cinema* cinema, Ticket* ticket, Screening* screening, int seat_id, time_t exchangingtime);

//fonction pour la gestion de la libération de place apres un visionnage
//int liberation_places(Screening* screening);

//retourne la disponibilite d'une salle
int is_roomavailable(Room* room);

//retourne la disponibilite d'une place
int is_seatavailable(Screening* screening, int id_seat);

//renvoi le statut de remplissage d'une salle
int is_roomfull(Room* room);

//Cree une liste de ticket
TicketList* ticketlist_create();

//fonction permettant de generer les statistiques d'un cinema
CinemaStatistics* generate_cinemastatistics(Cinema* cinema);
