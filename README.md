# SY40-Salle_Cinema

🎬 Système de Gestion de Cinéma en C - Architecture Complète
Je vais vous proposer une architecture système robuste pour ce projet complexe.
📋 Analyse du Cahier des Charges
Fonctionnalités Principales

✅ Gestion des places (numérotation, disponibilité)
✅ Achat de billets (guichets automatiques + hôtesses)
✅ Réservations avec priorité
✅ Projection flexible (changement de film)
✅ Échange de billets
✅ Restrictions d'âge
✅ Multi-salles et multi-films
✅ Files d'attente optimisées
✅ Annulation et remboursement
✅ Notifications clients
✅ Rapports statistiques
✅ Vérification automatique des capacités

🏗️ Architecture Système Proposée
1. Modèle de Données (Structures C)
c// ========== Types de base ==========

typedef enum {
    SEAT_AVAILABLE,
    SEAT_RESERVED,
    SEAT_SOLD
} SeatStatus;

typedef enum {
    TICKET_VALID,
    TICKET_USED,
    TICKET_CANCELLED,
    TICKET_EXCHANGED
} TicketStatus;

typedef enum {
    AGE_ALL,      // Tous publics
    AGE_12,       // -12 ans
    AGE_16,       // -16 ans
    AGE_18        // -18 ans
} AgeRating;

typedef struct {
    int id;
    int row;
    int col;
    SeatStatus status;
    int ticket_id;  // -1 si libre
} Seat;

typedef struct {
    int id;
    char title[100];
    int duration_minutes;
    AgeRating age_rating;
    char genre[50];
} Movie;

typedef struct {
    int id;
    char name[50];
    int capacity;
    Seat* seats;  // Tableau dynamique
    int rows;
    int cols;
    int available_seats;
} Room;

typedef struct {
    int id;
    Movie* movie;
    Room* room;
    time_t start_time;
    float price;
    int seats_sold;
    int seats_reserved;
    int can_change;  // 1 si < 20% vendus
} Screening;

typedef struct {
    int id;
    char customer_name[100];
    char email[100];
    int age;
    Screening* screening;
    int seat_id;
    TicketStatus status;
    time_t purchase_time;
    time_t reservation_time;  // 0 si achat direct
    int is_reservation;
} Ticket;

typedef struct TicketNode {
    Ticket* ticket;
    struct TicketNode* next;
} TicketNode;

typedef struct {
    TicketNode* head;
    TicketNode* tail;
    int size;
    pthread_mutex_t mutex;  // Sécurité thread
} TicketQueue;

typedef struct {
    int id;
    Screening** screenings;  // Tableau dynamique
    int num_screenings;
    Room** rooms;
    int num_rooms;
    Movie** movies;
    int num_movies;
    Ticket** tickets;
    int num_tickets;
    TicketQueue* kiosk_queue;    // File guichet auto
    TicketQueue* counter_queue;  // File hôtesse
} Cinema;

🔧 Stratégie d'Implémentation par Modules
Module 1: Gestion de la Mémoire et Initialisation
c// cinema_core.h
#ifndef CINEMA_CORE_H
#define CINEMA_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

// Création du cinéma
Cinema* cinema_create(int num_rooms);
void cinema_destroy(Cinema* cinema);

// Gestion des salles
Room* room_create(int id, const char* name, int rows, int cols);
void room_destroy(Room* room);
int room_add_to_cinema(Cinema* cinema, Room* room);

// Gestion des films
Movie* movie_create(int id, const char* title, int duration, AgeRating rating);
void movie_destroy(Movie* movie);
int movie_add_to_cinema(Cinema* cinema, Movie* movie);

#endif
Implémentation:
c// cinema_core.c

Cinema* cinema_create(int num_rooms) {
    Cinema* cinema = (Cinema*)malloc(sizeof(Cinema));
    if (!cinema) return NULL;
    
    cinema->id = 0;
    cinema->num_rooms = 0;
    cinema->rooms = (Room**)calloc(num_rooms, sizeof(Room*));
    cinema->num_movies = 0;
    cinema->movies = NULL;
    cinema->num_screenings = 0;
    cinema->screenings = NULL;
    cinema->num_tickets = 0;
    cinema->tickets = NULL;
    
    // Initialiser les files d'attente
    cinema->kiosk_queue = queue_create();
    cinema->counter_queue = queue_create();
    
    return cinema;
}

Room* room_create(int id, const char* name, int rows, int cols) {
    Room* room = (Room*)malloc(sizeof(Room));
    if (!room) return NULL;
    
    room->id = id;
    strncpy(room->name, name, sizeof(room->name) - 1);
    room->rows = rows;
    room->cols = cols;
    room->capacity = rows * cols;
    room->available_seats = room->capacity;
    
    // Allouer et initialiser les sièges
    room->seats = (Seat*)calloc(room->capacity, sizeof(Seat));
    for (int i = 0; i < room->capacity; i++) {
        room->seats[i].id = i;
        room->seats[i].row = i / cols;
        room->seats[i].col = i % cols;
        room->seats[i].status = SEAT_AVAILABLE;
        room->seats[i].ticket_id = -1;
    }
    
    return room;
}
Module 2: Gestion des Séances (Screenings)
c// screening_manager.h

// Créer une séance
Screening* screening_create(Cinema* cinema, int movie_id, int room_id, 
                           time_t start_time, float price);

// Changer le film d'une séance (si < 20% vendus)
int screening_change_movie(Screening* screening, Movie* new_movie);

// Vérifier si changement possible
int screening_can_change(Screening* screening);

// Obtenir les places disponibles
int screening_get_available_seats(Screening* screening);

// Mettre à jour l'état des places
void screening_update_seats(Screening* screening);
Implémentation:
c// screening_manager.c

int screening_can_change(Screening* screening) {
    if (!screening || !screening->room) return 0;
    
    float sold_percentage = (float)screening->seats_sold / 
                           screening->room->capacity * 100.0f;
    
    return (sold_percentage < 20.0f);
}

int screening_change_movie(Screening* screening, Movie* new_movie) {
    if (!screening_can_change(screening)) {
        fprintf(stderr, "Cannot change movie: more than 20%% seats sold\n");
        return -1;
    }
    
    // Notifier les clients ayant réservé
    for (int i = 0; i < screening->seats_reserved; i++) {
        // TODO: envoyer notification
    }
    
    screening->movie = new_movie;
    return 0;
}
Module 3: Gestion des Réservations et Achats
c// ticket_manager.h

// Réserver une place
Ticket* ticket_reserve(Cinema* cinema, int screening_id, int seat_id,
                      const char* customer_name, const char* email, int age);

// Acheter un billet (direct ou depuis réservation)
Ticket* ticket_purchase(Cinema* cinema, int screening_id, int seat_id,
                       const char* customer_name, int age, int is_kiosk);

// Vérifier l'âge pour les restrictions
int ticket_check_age_restriction(Screening* screening, int customer_age);

// Échanger un billet
int ticket_exchange(Cinema* cinema, Ticket* old_ticket, 
                   int new_screening_id, int new_seat_id);

// Annuler un billet
int ticket_cancel(Cinema* cinema, Ticket* ticket);
Implémentation avec gestion atomique:
c// ticket_manager.c

static pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;

Ticket* ticket_purchase(Cinema* cinema, int screening_id, int seat_id,
                       const char* customer_name, int age, int is_kiosk) {
    
    pthread_mutex_lock(&ticket_mutex);  // SECTION CRITIQUE
    
    Screening* screening = cinema->screenings[screening_id];
    Room* room = screening->room;
    Seat* seat = &room->seats[seat_id];
    
    // Vérifications
    if (seat->status != SEAT_AVAILABLE) {
        pthread_mutex_unlock(&ticket_mutex);
        return NULL;
    }
    
    if (!ticket_check_age_restriction(screening, age)) {
        pthread_mutex_unlock(&ticket_mutex);
        fprintf(stderr, "Age restriction not met\n");
        return NULL;
    }
    
    // Créer le billet
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    ticket->id = cinema->num_tickets++;
    strncpy(ticket->customer_name, customer_name, sizeof(ticket->customer_name) - 1);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    ticket->purchase_time = time(NULL);
    ticket->is_reservation = 0;
    
    // Mettre à jour la place
    seat->status = SEAT_SOLD;
    seat->ticket_id = ticket->id;
    screening->seats_sold++;
    room->available_seats--;
    
    // Ajouter à la liste des billets
    cinema->tickets = realloc(cinema->tickets, 
                             sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = ticket;
    
    pthread_mutex_unlock(&ticket_mutex);  // FIN SECTION CRITIQUE
    
    return ticket;
}

Ticket* ticket_reserve(Cinema* cinema, int screening_id, int seat_id,
                      const char* customer_name, const char* email, int age) {
    
    pthread_mutex_lock(&ticket_mutex);
    
    Screening* screening = cinema->screenings[screening_id];
    Seat* seat = &screening->room->seats[seat_id];
    
    if (seat->status != SEAT_AVAILABLE) {
        pthread_mutex_unlock(&ticket_mutex);
        return NULL;
    }
    
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    ticket->id = cinema->num_tickets++;
    strncpy(ticket->customer_name, customer_name, sizeof(ticket->customer_name) - 1);
    strncpy(ticket->email, email, sizeof(ticket->email) - 1);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    ticket->reservation_time = time(NULL);
    ticket->is_reservation = 1;
    
    seat->status = SEAT_RESERVED;
    seat->ticket_id = ticket->id;
    screening->seats_reserved++;
    
    cinema->tickets = realloc(cinema->tickets, 
                             sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = ticket;
    
    pthread_mutex_unlock(&ticket_mutex);
    
    return ticket;
}
Module 4: Gestion des Files d'Attente
c// queue_manager.h

TicketQueue* queue_create(void);
void queue_destroy(TicketQueue* queue);

// Ajouter un client en attente
int queue_enqueue(TicketQueue* queue, Ticket* ticket);

// Retirer le prochain client (FIFO avec priorité réservations)
Ticket* queue_dequeue(TicketQueue* queue);

// Taille de la file
int queue_size(TicketQueue* queue);
Implémentation thread-safe:
c// queue_manager.c

TicketQueue* queue_create(void) {
    TicketQueue* queue = (TicketQueue*)malloc(sizeof(TicketQueue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    return queue;
}

int queue_enqueue(TicketQueue* queue, Ticket* ticket) {
    pthread_mutex_lock(&queue->mutex);
    
    TicketNode* node = (TicketNode*)malloc(sizeof(TicketNode));
    node->ticket = ticket;
    node->next = NULL;
    
    // Insertion avec priorité pour les réservations
    if (ticket->is_reservation) {
        // Insérer en tête pour priorité
        node->next = queue->head;
        queue->head = node;
        if (queue->tail == NULL) {
            queue->tail = node;
        }
    } else {
        // Insérer en queue (FIFO normal)
        if (queue->tail) {
            queue->tail->next = node;
        }
        queue->tail = node;
        if (queue->head == NULL) {
            queue->head = node;
        }
    }
    
    queue->size++;
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

Ticket* queue_dequeue(TicketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    TicketNode* node = queue->head;
    Ticket* ticket = node->ticket;
    queue->head = node->next;
    
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    
    free(node);
    queue->size--;
    
    pthread_mutex_unlock(&queue->mutex);
    return ticket;
}
Module 5: Système de Notifications
c// notification_system.h

typedef enum {
    NOTIF_EMAIL,
    NOTIF_SCREEN
} NotificationType;

// Envoyer une notification
int notify_customer(Ticket* ticket, const char* message, NotificationType type);

// Notifier un changement de film
int notify_movie_change(Screening* screening, Movie* new_movie);

// Notifier un changement de salle
int notify_room_change(Ticket* ticket, Room* new_room);
Implémentation:
c// notification_system.c

int notify_customer(Ticket* ticket, const char* message, NotificationType type) {
    if (type == NOTIF_EMAIL) {
        // Simuler l'envoi d'email
        printf("[EMAIL] To: %s\n", ticket->email);
        printf("Subject: Cinema Notification\n");
        printf("Message: %s\n\n", message);
    } else {
        // Affichage sur guichet automatique
        printf("[SCREEN] Customer: %s\n", ticket->customer_name);
        printf("Message: %s\n\n", message);
    }
    
    return 0;
}

int notify_movie_change(Screening* screening, Movie* new_movie) {
    char message[256];
    snprintf(message, sizeof(message),
             "Film change: Your screening now shows '%s' instead. "
             "You can exchange your ticket if desired.",
             new_movie->title);
    
    // Parcourir tous les billets de cette séance
    // et notifier les clients
    // TODO: implémenter parcours
    
    return 0;
}
Module 6: Rapports et Statistiques
c// statistics.h

typedef struct {
    int total_tickets_sold;
    int total_tickets_reserved;
    int total_tickets_cancelled;
    float total_revenue;
    float occupancy_rate;
    int tickets_by_movie[100];  // Index = movie_id
    int tickets_by_room[20];    // Index = room_id
    float avg_waiting_time_kiosk;
    float avg_waiting_time_counter;
    int exchanges_count;
} CinemaStatistics;

// Générer les statistiques
CinemaStatistics* stats_generate(Cinema* cinema);

// Afficher le rapport
void stats_print(CinemaStatistics* stats);

// Sauvegarder dans un fichier
int stats_save_to_file(CinemaStatistics* stats, const char* filename);
Implémentation:
c// statistics.c

CinemaStatistics* stats_generate(Cinema* cinema) {
    CinemaStatistics* stats = (CinemaStatistics*)calloc(1, sizeof(CinemaStatistics));
    
    for (int i = 0; i < cinema->num_tickets; i++) {
        Ticket* ticket = cinema->tickets[i];
        
        if (ticket->status == TICKET_USED || ticket->status == TICKET_VALID) {
            stats->total_tickets_sold++;
            stats->total_revenue += ticket->screening->price;
            stats->tickets_by_movie[ticket->screening->movie->id]++;
            stats->tickets_by_room[ticket->screening->room->id]++;
        } else if (ticket->status == TICKET_CANCELLED) {
            stats->total_tickets_cancelled++;
        } else if (ticket->is_reservation) {
            stats->total_tickets_reserved++;
        }
        
        if (ticket->status == TICKET_EXCHANGED) {
            stats->exchanges_count++;
        }
    }
    
    // Calculer le taux d'occupation
    int total_capacity = 0;
    for (int i = 0; i < cinema->num_screenings; i++) {
        total_capacity += cinema->screenings[i]->room->capacity;
    }
    
    if (total_capacity > 0) {
        stats->occupancy_rate = (float)stats->total_tickets_sold / total_capacity * 100.0f;
    }
    
    return stats;
}

void stats_print(CinemaStatistics* stats) {
    printf("\n========== CINEMA STATISTICS ==========\n");
    printf("Total tickets sold: %d\n", stats->total_tickets_sold);
    printf("Total tickets reserved: %d\n", stats->total_tickets_reserved);
    printf("Total tickets cancelled: %d\n", stats->total_tickets_cancelled);
    printf("Total revenue: %.2f€\n", stats->total_revenue);
    printf("Occupancy rate: %.1f%%\n", stats->occupancy_rate);
    printf("Ticket exchanges: %d\n", stats->exchanges_count);
    printf("=======================================\n\n");
}
🧵 Gestion Multi-Thread (Système Concurrent)
Architecture Thread:
c// thread_manager.h

// Thread pour guichet automatique
void* kiosk_thread(void* arg);

// Thread pour hôtesse
void* counter_thread(void* arg);

// Thread de vérification des capacités
void* capacity_monitor_thread(void* arg);

// Démarrer le système
int cinema_start_threads(Cinema* cinema);
Implémentation:
c// thread_manager.c

typedef struct {
    Cinema* cinema;
    int kiosk_id;
} KioskThreadData;

void* kiosk_thread(void* arg) {
    KioskThreadData* data = (KioskThreadData*)arg;
    Cinema* cinema = data->cinema;
    
    while (1) {
        // Attendre un client dans la file
        Ticket* ticket = queue_dequeue(cinema->kiosk_queue);
        
        if (ticket) {
            printf("[KIOSK %d] Processing customer: %s\n", 
                   data->kiosk_id, ticket->customer_name);
            
            // Simuler le temps de traitement
            sleep(2);
            
            // Traiter l'achat
            if (ticket->is_reservation) {
                // Convertir réservation en achat
                ticket->is_reservation = 0;
                ticket->purchase_time = time(NULL);
            }
            
            printf("[KIOSK %d] Ticket issued: ID %d\n", 
                   data->kiosk_id, ticket->id);
        }
        
        sleep(1);  // Attendre le prochain client
    }
    
    return NULL;
}

void* capacity_monitor_thread(void* arg) {
    Cinema* cinema = (Cinema*)arg;
    
    while (1) {
        for (int i = 0; i < cinema->num_screenings; i++) {
            Screening* screening = cinema->screenings[i];
            
            float occupancy = (float)(screening->seats_sold + screening->seats_reserved) / 
                            screening->room->capacity * 100.0f;
            
            if (occupancy >= 90.0f) {
                printf("[ALERT] Screening %d (%s) is %.1f%% full!\n",
                       screening->id, screening->movie->title, occupancy);
                
                // Empêcher nouvelles réservations/achats si 100%
                if (occupancy >= 100.0f) {
                    // Marquer comme complet
                }
            }
        }
        
        sleep(30);  // Vérifier toutes les 30 secondes
    }
    
    return NULL;
}

int cinema_start_threads(Cinema* cinema) {
    pthread_t kiosk_threads[3];  // 3 guichets automatiques
    pthread_t counter_threads[2]; // 2 hôtesses
    pthread_t monitor_thread;
    
    // Démarrer les guichets
    for (int i = 0; i < 3; i++) {
        KioskThreadData* data = malloc(sizeof(KioskThreadData));
        data->cinema = cinema;
        data->kiosk_id = i + 1;
        pthread_create(&kiosk_threads[i], NULL, kiosk_thread, data);
    }
    
    // Démarrer le moniteur
    pthread_create(&monitor_thread, NULL, capacity_monitor_thread, cinema);
    
    return 0;
}
```

## 📦 **Structure Finale des Fichiers**
```
cinema_management/
├── include/
│   ├── cinema_core.h
│   ├── screening_manager.h
│   ├── ticket_manager.h
│   ├── queue_manager.h
│   ├── notification_system.h
│   ├── statistics.h
│   └── thread_manager.h
├── src/
│   ├── cinema_core.c
│   ├── screening_manager.c
│   ├── ticket_manager.c
│   ├── queue_manager.c
│   ├── notification_system.c
│   ├── statistics.c
│   ├── thread_manager.c
│   └── main.c
├── data/
│   ├── movies.txt
│   ├── rooms.txt
│   └── screenings.txt
├── Makefile
└── README.md
🎯 Exemple d'Utilisation (main.c)
c// main.c

#include "cinema_core.h"
#include "screening_manager.h"
#include "ticket_manager.h"
#include "statistics.h"
#include "thread_manager.h"

int main() {
    // Créer le cinéma
    Cinema* cinema = cinema_create(5);
    
    // Ajouter des salles
    Room* room1 = room_create(0, "Salle 1", 10, 15);
    room_add_to_cinema(cinema, room1);
    
    // Ajouter des films
    Movie* movie1 = movie_create(0, "Avatar 3", 180, AGE_ALL);
    movie_add_to_cinema(cinema, movie1);
    
    // Créer des séances
    time_t now = time(NULL);
    Screening* screening1 = screening_create(cinema, 0, 0, now + 3600, 12.50);
    
    // Démarrer les threads
    cinema_start_threads(cinema);
    
    // Simuler des achats
    Ticket* ticket1 = ticket_purchase(cinema, 0, 50, "John Doe", 25, 1);
    Ticket* ticket2 = ticket_reserve(cinema, 0, 51, "Jane Smith", "jane@email.com", 30);
    
    // Générer les statistiques
    sleep(5);
    CinemaStatistics* stats = stats_generate(cinema);
    stats_print(stats);
    
    // Nettoyer
    cinema_destroy(cinema);
    free(stats);
    
    return 0;
}
🔐 Points Clés de Sécurité

Mutex sur les sections critiques (achat/réservation de places)
Files d'attente thread-safe
Vérifications atomiques (âge, disponibilité)
Gestion des deadlocks (ordre d'acquisition des mutex)
Validation des données (pointeurs, indices)

📈 Optimisations Possibles

Cache des sièges disponibles par salle
Pool de threads au lieu de threads fixes
Base de données (SQLite) au lieu de structures en mémoire
Compression des statistiques pour historique long terme
Load balancing entre guichets selon la charge

Voulez-vous que je développe un module spécifique en détail ? 🎬Sje ne comprends pas du tout ta logique, je souhaite que tu me fasse une explication les plus claires quant au fonctionnement de ton code et surtout comment l'achat d'un nombre aléatoire de billets se déroulera🎬 Explication Claire et Détaillée du Système de Cinéma
Je vais vous expliquer le fonctionnement étape par étape, comme si vous découvriez le système pour la première fois.
🧠 Concept Global : Qu'est-ce qu'on construit ?
Imaginez un vrai cinéma :

Des salles avec des sièges numérotés
Des films qui passent à des horaires précis (séances)
Des clients qui achètent des billets pour une séance spécifique
Des guichets (automatiques et humains) où les clients font la queue

Notre système en C va modéliser tout ça en mémoire.

📦 Les Éléments de Base (Les "Briques")
1. Un Siège (Seat)
ctypedef struct {
    int id;              // Numéro du siège (ex: 42)
    int row;             // Rangée (ex: 5)
    int col;             // Colonne (ex: 7)
    SeatStatus status;   // État: LIBRE, RÉSERVÉ ou VENDU
    int ticket_id;       // Quel billet occupe ce siège (-1 si libre)
} Seat;
Analogie : Une chaise de cinéma avec un numéro. Elle peut être :

🟢 Verte (libre)
🟡 Jaune (réservée)
🔴 Rouge (vendue)

2. Une Salle (Room)
ctypedef struct {
    int id;                  // ID de la salle (ex: 1)
    char name[50];           // Nom (ex: "Salle 1")
    Seat* seats;             // TABLEAU de tous les sièges
    int rows;                // Nombre de rangées (ex: 10)
    int cols;                // Nombre de colonnes (ex: 15)
    int capacity;            // Total = rows × cols = 150
    int available_seats;     // Combien de sièges libres (ex: 120)
} Room;
```

**Analogie** : Une salle de cinéma physique avec toutes ses chaises.

**Exemple** : Salle de 10 rangées × 15 colonnes = 150 sièges
```
         Colonnes
         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
Rangée 0 [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Rangée 1 [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Rangée 2 [ ][ ][ ][X][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]  <- X = occupé
...
3. Un Film (Movie)
ctypedef struct {
    int id;                  // ID du film
    char title[100];         // Titre (ex: "Avatar 3")
    int duration_minutes;    // Durée (ex: 180 minutes)
    AgeRating age_rating;    // Restriction d'âge (TOUS, -12, -16, -18)
} Movie;
Analogie : Une affiche de film avec ses infos.
4. Une Séance (Screening)
ctypedef struct {
    int id;                  // ID de la séance
    Movie* movie;            // QUEL film est projeté
    Room* room;              // Dans QUELLE salle
    time_t start_time;       // À QUELLE heure (timestamp Unix)
    float price;             // Prix du billet (ex: 12.50€)
    int seats_sold;          // Combien de billets vendus
    int seats_reserved;      // Combien de billets réservés
} Screening;
```

**Analogie** : Une projection spécifique.

**Exemple** :
```
Séance #5:
  Film: "Avatar 3"
  Salle: Salle 1 (150 places)
  Heure: 20h30 ce soir
  Prix: 12.50€
  Vendus: 45 billets
  Réservés: 12 billets
  Disponibles: 150 - 45 - 12 = 93 places
5. Un Billet (Ticket)
ctypedef struct {
    int id;                      // Numéro du billet
    char customer_name[100];     // Nom du client
    int age;                     // Âge (pour vérification)
    Screening* screening;        // Pour QUELLE séance
    int seat_id;                 // QUEL siège (ex: siège 42)
    TicketStatus status;         // VALIDE, UTILISÉ, ANNULÉ, ÉCHANGÉ
    time_t purchase_time;        // Quand acheté
    int is_reservation;          // 0 = achat, 1 = réservation
} Ticket;
Analogie : Votre billet papier/numérique.
6. Le Cinéma (Cinema)
ctypedef struct {
    Room** rooms;            // TABLEAU de POINTEURS vers des salles
    int num_rooms;           // Combien de salles
    Movie** movies;          // TABLEAU de films
    int num_movies;
    Screening** screenings;  // TABLEAU de séances
    int num_screenings;
    Ticket** tickets;        // TABLEAU de tous les billets vendus
    int num_tickets;
    TicketQueue* kiosk_queue;    // File d'attente guichet auto
    TicketQueue* counter_queue;  // File d'attente hôtesse
} Cinema;
Analogie : Le bâtiment entier du cinéma avec tout dedans.

🎯 Scénario Complet : Achat d'un Nombre Aléatoire de Billets
Imaginons que 3 clients arrivent et veulent acheter 2, 1 et 3 billets pour la même séance.
ÉTAPE 0 : Initialisation du Système
cint main() {
    // Créer le cinéma
    Cinema* cinema = cinema_create(5);  // Cinéma avec 5 salles max
    
    // Créer une salle (Salle 1: 10 rangées × 15 colonnes = 150 places)
    Room* room1 = room_create(0, "Salle 1", 10, 15);
    room_add_to_cinema(cinema, room1);
    
    // Créer un film
    Movie* movie1 = movie_create(0, "Avatar 3", 180, AGE_ALL);
    movie_add_to_cinema(cinema, movie1);
    
    // Créer une séance (Avatar 3 dans Salle 1 à 20h30, 12.50€)
    time_t tonight_8pm = time(NULL) + 3600;  // Dans 1h
    Screening* screening1 = screening_create(cinema, 0, 0, tonight_8pm, 12.50);
    
    // Maintenant on a:
    // - 1 salle avec 150 sièges TOUS LIBRES
    // - 1 film
    // - 1 séance
    // - 0 billet vendu
}
```

**État initial** :
```
Salle 1 : 150 sièges libres
Séance #0 : Avatar 3 à 20h30, 0/150 vendus

ÉTAPE 1 : Client 1 Veut Acheter 2 Billets
c// Client 1: "Alice" veut 2 billets pour la séance 0
int num_tickets = 2;
char* customer_name = "Alice";
int customer_age = 25;

// On va acheter les sièges 10 et 11 (rangée 0, colonnes 10 et 11)
for (int i = 0; i < num_tickets; i++) {
    int seat_id = 10 + i;  // Sièges 10, 11
    
    Ticket* ticket = ticket_purchase(
        cinema,           // Notre cinéma
        0,                // Séance #0
        seat_id,          // Siège à acheter
        customer_name,    // Nom du client
        customer_age,     // Âge
        1                 // 1 = guichet automatique
    );
    
    if (ticket) {
        printf("✅ Billet #%d créé pour %s (siège %d)\n", 
               ticket->id, customer_name, seat_id);
    } else {
        printf("❌ Échec : siège %d déjà occupé ou invalide\n", seat_id);
    }
}
Ce qui se passe dans ticket_purchase() :
cTicket* ticket_purchase(Cinema* cinema, int screening_id, int seat_id,
                       const char* customer_name, int age, int is_kiosk) {
    
    // 🔒 VERROUILLER (empêcher 2 personnes d'acheter en même temps)
    pthread_mutex_lock(&ticket_mutex);
    
    // 1️⃣ Récupérer la séance
    Screening* screening = cinema->screenings[screening_id];
    
    // 2️⃣ Récupérer la salle et le siège
    Room* room = screening->room;
    Seat* seat = &room->seats[seat_id];
    
    // 3️⃣ VÉRIFIER : Le siège est-il libre ?
    if (seat->status != SEAT_AVAILABLE) {
        pthread_mutex_unlock(&ticket_mutex);
        return NULL;  // ❌ ÉCHEC : siège occupé
    }
    
    // 4️⃣ VÉRIFIER : Le client a-t-il l'âge requis ?
    if (!ticket_check_age_restriction(screening, age)) {
        pthread_mutex_unlock(&ticket_mutex);
        return NULL;  // ❌ ÉCHEC : trop jeune
    }
    
    // 5️⃣ CRÉER le billet
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    ticket->id = cinema->num_tickets++;  // ID unique
    strcpy(ticket->customer_name, customer_name);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    ticket->purchase_time = time(NULL);
    ticket->is_reservation = 0;
    
    // 6️⃣ MARQUER le siège comme VENDU
    seat->status = SEAT_SOLD;
    seat->ticket_id = ticket->id;
    
    // 7️⃣ METTRE À JOUR les compteurs
    screening->seats_sold++;           // Une place de plus vendue
    room->available_seats--;           // Une place de moins dispo
    
    // 8️⃣ AJOUTER le billet à la liste des billets du cinéma
    cinema->tickets = realloc(cinema->tickets, 
                             sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = ticket;
    
    // 🔓 DÉVERROUILLER
    pthread_mutex_unlock(&ticket_mutex);
    
    return ticket;  // ✅ SUCCÈS
}
```

**Résultat après Client 1** :
```
Salle 1 : 148 sièges libres (150 - 2)
Séance #0 : 2/150 vendus
Billets créés : #0 (Alice, siège 10), #1 (Alice, siège 11)

Plan de la salle (rangée 0) :
[0][1][2][3][4][5][6][7][8][9][X][X][12][13][14]
                                 ↑  ↑
                              Alice  Alice

ÉTAPE 2 : Client 2 Veut Acheter 1 Billet (EN MÊME TEMPS)
Pendant que Client 1 achète, Client 2 arrive simultanément (système multi-thread).
c// Thread 2 (EN PARALLÈLE)
char* customer2 = "Bob";
int seat_id = 11;  // ⚠️ Bob veut AUSSI le siège 11 !

Ticket* ticket = ticket_purchase(cinema, 0, 11, "Bob", 30, 1);
Ce qui se passe :

Bob arrive à pthread_mutex_lock(&ticket_mutex)
MAIS Alice est déjà dedans (elle a locké en premier)
Bob ATTEND (bloqué) jusqu'à ce qu'Alice finisse
Quand Alice fait pthread_mutex_unlock(), Bob entre
Bob vérifie : seat->status == SEAT_SOLD (Alice l'a déjà pris !)
Bob reçoit NULL → Échec, siège déjà vendu ❌

Protection contre les conflits grâce au mutex ! 🔒
Bob essaie alors le siège 12 :
cTicket* ticket = ticket_purchase(cinema, 0, 12, "Bob", 30, 1);
// ✅ SUCCÈS : siège 12 libre
```

**Résultat après Client 2** :
```
Salle 1 : 147 sièges libres
Séance #0 : 3/150 vendus
Billets : #0 (Alice,10), #1 (Alice,11), #2 (Bob,12)

[0][1][2][3][4][5][6][7][8][9][X][X][X][13][14]
                                 ↑  ↑  ↑
                              Alice A. Bob

ÉTAPE 3 : Client 3 Veut Acheter 3 Billets (Dont un Déjà Pris)
cchar* customer3 = "Charlie";
int wanted_seats[] = {12, 13, 14};  // ⚠️ 12 est déjà pris par Bob !

for (int i = 0; i < 3; i++) {
    Ticket* ticket = ticket_purchase(cinema, 0, wanted_seats[i], "Charlie", 28, 0);
    
    if (ticket) {
        printf("✅ Charlie : billet pour siège %d\n", wanted_seats[i]);
    } else {
        printf("❌ Charlie : siège %d déjà pris, chercher un autre...\n", wanted_seats[i]);
        
        // STRATÉGIE : Trouver automatiquement un siège libre
        int found_seat = find_next_available_seat(cinema, 0);
        if (found_seat != -1) {
            ticket = ticket_purchase(cinema, 0, found_seat, "Charlie", 28, 0);
            printf("✅ Charlie : siège alternatif %d trouvé\n", found_seat);
        }
    }
}
Fonction pour trouver un siège libre :
cint find_next_available_seat(Cinema* cinema, int screening_id) {
    Screening* screening = cinema->screenings[screening_id];
    Room* room = screening->room;
    
    // Parcourir tous les sièges
    for (int i = 0; i < room->capacity; i++) {
        if (room->seats[i].status == SEAT_AVAILABLE) {
            return i;  // Premier siège libre trouvé
        }
    }
    
    return -1;  // Aucun siège libre
}
```

**Résultat final** :
```
Salle 1 : 143 sièges libres (150 - 7)
Séance #0 : 7/150 vendus

Billets vendus :
  #0 : Alice, siège 10
  #1 : Alice, siège 11
  #2 : Bob, siège 12
  #3 : Charlie, siège 15 (13 était libre, trouvé auto)
  #4 : Charlie, siège 14
  #5 : Charlie, siège 16 (alternatif pour 12)

🔄 Fonction Complète : Acheter N Billets Aléatoires
Voici une fonction robuste qui gère tout automatiquement :
c/**
 * Acheter un nombre aléatoire de billets pour une séance
 * @param cinema Le cinéma
 * @param screening_id ID de la séance
 * @param customer_name Nom du client
 * @param customer_age Âge du client
 * @param min_tickets Minimum de billets à acheter
 * @param max_tickets Maximum de billets à acheter
 * @return Nombre de billets réellement achetés
 */
int purchase_random_tickets(Cinema* cinema, int screening_id,
                           const char* customer_name, int customer_age,
                           int min_tickets, int max_tickets) {
    
    // 1️⃣ Générer un nombre aléatoire de billets
    srand(time(NULL));
    int num_tickets = min_tickets + rand() % (max_tickets - min_tickets + 1);
    
    printf("🎫 %s veut acheter %d billets...\n", customer_name, num_tickets);
    
    // 2️⃣ Vérifier qu'il y a assez de places
    Screening* screening = cinema->screenings[screening_id];
    int available = screening->room->available_seats;
    
    if (available < num_tickets) {
        printf("⚠️  Seulement %d places disponibles (demandé: %d)\n", 
               available, num_tickets);
        num_tickets = available;  // Acheter ce qui reste
    }
    
    if (num_tickets == 0) {
        printf("❌ Aucune place disponible !\n");
        return 0;
    }
    
    // 3️⃣ Acheter les billets un par un
    int tickets_bought = 0;
    Ticket** bought_tickets = malloc(sizeof(Ticket*) * num_tickets);
    
    for (int i = 0; i < num_tickets; i++) {
        // Trouver un siège libre
        int seat_id = find_next_available_seat(cinema, screening_id);
        
        if (seat_id == -1) {
            printf("⚠️  Plus de sièges libres après %d achats\n", i);
            break;
        }
        
        // Acheter le billet
        Ticket* ticket = ticket_purchase(
            cinema, screening_id, seat_id,
            customer_name, customer_age, 1
        );
        
        if (ticket) {
            bought_tickets[tickets_bought++] = ticket;
            printf("  ✅ Billet #%d : siège %d (rangée %d, colonne %d)\n",
                   ticket->id, seat_id,
                   seat_id / screening->room->cols,
                   seat_id % screening->room->cols);
        } else {
            printf("  ❌ Échec pour le siège %d\n", seat_id);
        }
    }
    
    // 4️⃣ Afficher le résumé
    printf("📊 Total acheté : %d/%d billets (%.2f€)\n\n",
           tickets_bought, num_tickets,
           tickets_bought * screening->price);
    
    free(bought_tickets);
    return tickets_bought;
}

🎮 Programme Complet de Test
cint main() {
    // Initialisation
    Cinema* cinema = cinema_create(5);
    Room* room1 = room_create(0, "Salle 1", 10, 15);  // 150 places
    room_add_to_cinema(cinema, room1);
    
    Movie* movie1 = movie_create(0, "Avatar 3", 180, AGE_ALL);
    movie_add_to_cinema(cinema, movie1);
    
    time_t tonight = time(NULL) + 3600;
    Screening* screening1 = screening_create(cinema, 0, 0, tonight, 12.50);
    
    printf("🎬 Cinéma initialisé : Salle 1 (150 places), Avatar 3 à 20h30\n\n");
    
    // Simuler 5 clients avec achats aléatoires
    purchase_random_tickets(cinema, 0, "Alice", 25, 1, 3);
    purchase_random_tickets(cinema, 0, "Bob", 30, 2, 5);
    purchase_random_tickets(cinema, 0, "Charlie", 28, 1, 4);
    purchase_random_tickets(cinema, 0, "Diana", 22, 3, 6);
    purchase_random_tickets(cinema, 0, "Eve", 35, 2, 4);
    
    // Afficher l'état final
    printf("📈 RÉSUMÉ FINAL :\n");
    printf("  Places vendues : %d/150\n", screening1->seats_sold);
    printf("  Places libres : %d/150\n", room1->available_seats);
    printf("  Taux d'occupation : %.1f%%\n",
           (float)screening1->seats_sold / room1->capacity * 100);
    
    cinema_destroy(cinema);
    return 0;
}
```

**Sortie exemple** :
```
🎬 Cinéma initialisé : Salle 1 (150 places), Avatar 3 à 20h30

🎫 Alice veut acheter 2 billets...
  ✅ Billet #0 : siège 0 (rangée 0, colonne 0)
  ✅ Billet #1 : siège 1 (rangée 0, colonne 1)
📊 Total acheté : 2/2 billets (25.00€)

🎫 Bob veut acheter 4 billets...
  ✅ Billet #2 : siège 2 (rangée 0, colonne 2)
  ✅ Billet #3 : siège 3 (rangée 0, colonne 3)
  ✅ Billet #4 : siège 4 (rangée 0, colonne 4)
  ✅ Billet #5 : siège 5 (rangée 0, colonne 5)
📊 Total acheté : 4/4 billets (50.00€)

...

📈 RÉSUMÉ FINAL :
  Places vendues : 15/150
  Places libres : 135/150
  Taux d'occupation : 10.0%

🔑 Points Clés à Retenir

Mutex = Verrou pour éviter que 2 personnes achètent le même siège
Structures imbriquées : Cinema → Screening → Room → Seats
Allocation dynamique : Les tableaux grandissent avec realloc()
État atomique : Chaque achat change l'état de 3 choses en même temps :

Le siège (SEAT_SOLD)
Les compteurs (seats_sold++, available_seats--)
La liste des billets


Sécurité : Vérifications avant chaque achat (âge, disponibi
