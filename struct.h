//
// Created by kysyb on 19/12/2025.
//
#include <pthread.h>

typedef enum { SEAT_AVAILABLE,
               SEAT_RESERVED,
               SEAT_SOLD
             } SeatStatus; //permet de definir la statut d'une place

typedef enum { TICKET_VALID,
               TICKET_USED,
               TICKET_CANCELLED,
               TICKET_EXCHANGED
             } TicketStatus;

typedef enum { AGE_ALL, // Tous publics
               AGE_12, // Deconseille aux moins de 12 ans
               AGE_16, // Deconseille au moins de 16 ans
               AGE_18 // Deconseille au moins de18 ans
             } AgeRating;

typedef struct { int id;
                 int row;
                 int col;
                 SeatStatus status;
                 int ticket_id; // initialiser a -1 si le siege est libre
               } Seat;

typedef struct { int id;
                 char title[100];
                 int duration_minutes;
                 AgeRating age_rating;
                 char genre[50];
               } Movie;

typedef struct { int id;
                 char name[50];
                 int capacity;
                 Seat* seats; // Tableau dynamique contenant tous les sieges de la salle
                 int rows;
                 int cols;
                 int available_seats;
               } Room;

typedef struct { int id;
                 Movie* movie;
                 Room* room;
                 // date du film
                 time_t start_time;
                 float price;
                 int seats_sold;
                 int seats_reserved;
                 int can_change; // prendra la valeur 1 si seats_sold < 20%
               } Screening;

typedef struct { int id;
                 char customer_name[50];
                 char email[50];
                 int age;
                 Screening* screening;
                 int seat_id;
                 TicketStatus status;
                 // date de la transaction
                 time_t purchase_time;
                 time_t reservation_time; // 0 si achat direct
                 int is_reservation; //permet de savoir si c'est une reservation de billet ou non
               } Ticket;

typedef struct TicketNode { Ticket* ticket;
                            struct TicketNode* next;
                          } TicketNode;

typedef struct { TicketNode* head;
                 TicketNode* actual;
                 TicketNode* tail;
                 int size;
                 pthread_mutex_t mutex; // Sécurité thread
               } TicketList;

typedef struct { int id;
                 Screening** screenings; // Tableau dynamique contenant toutes les seances de visionnage du cinema
                 int num_screenings;
                 Room** rooms;
                 int num_rooms;
                 Movie** movies;
                 int num_movies;
                 Ticket** tickets;
                 int num_tickets;
                 TicketList* kiosk_list; // liste de ticket du guichet automatique
                 TicketList* counter_list; // queue de l'hotesse
               } Cinema;
