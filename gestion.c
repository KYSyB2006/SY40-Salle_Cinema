#include "gestion.h"

static volatile int id_cinema=0;
static volatile int id_room=0;
static volatile int id_screening=0;

// creation du cinema
Cinema* cinema_create(int num_rooms_total){
    Cinema* cinema = malloc(sizeof(Cinema));
    if (!cinema) return NULL;

    // Initialisation des champs du cinema
    cinema->id = id_cinema++;
    cinema->num_rooms = 0;
    cinema->rooms = calloc(num_rooms_total, sizeof(Room*));
    cinema->num_movies = 0;
    cinema->movies = NULL;
    cinema->num_screenings = 0;
    cinema->screenings = NULL;
    cinema->num_tickets = 0;
    cinema->tickets = NULL;
    cinema->statistics = malloc(sizeof(CinemaStatistics));
    if (!cinema->statistics) return NULL;
    //initalisation des statistiques à 0
    memset(cinema->statistics, 0, sizeof(CinemaStatistics));
    cinema->kiosk_list = ticketlist_create();
    cinema->counter_list = ticketlist_create();
    return cinema;
}

//creation d'un room
Room* room_create(const char* name, int rows, int cols) {
    Room* room = (Room*)malloc(sizeof(Room));
    if (!room) return NULL;

    room->id = id_room++;
    strncpy(room->name, name, sizeof(room->name) - 1);
    room->rows = rows;
    room->cols = cols;
    room->capacity = rows * cols;
    room->available_seats = room->capacity;
    room->for_event=0;

    // Allocation et initialisation des sièges
    room->seats = malloc(sizeof(Seat*) * room->capacity);
    for (int i = 0; i < room->capacity; i++) {
        room->seats[i] = malloc(sizeof(Seat));
        room->seats[i]->id = i;
        room->seats[i]->row = i / cols;
        room->seats[i]->col = i % cols;
        room->seats[i]->status = SEAT_AVAILABLE;
        room->seats[i]->ticket_id = -1;
    }

    return room;
}

//creation d'un movie
Movie* movie_create(int id, const char* title, int duration, AgeRating rating, char* genre)
{
    Movie* movie = (Movie*)malloc(sizeof(Movie));
    if (!movie) return NULL;

    movie->id = id;
    strncpy(movie->title, title, sizeof(movie->title) - 1);
    movie->duration_minutes = duration;
    movie->age_rating = rating;
    strncpy(movie->genre, genre, sizeof(movie->genre) - 1);

    return movie;

}

//creation d'un screening
Screening* screening_create(Movie* movie, Room* room, time_t start_time, float price)
{
    Screening* screening = (Screening*)malloc(sizeof(Screening));
    if (!screening) return NULL;

    screening->id = id_screening++;
    screening->movie = movie;
    screening->room = room;
    screening->start_time = start_time;
    screening->price = price;
    screening->seats_sold = 0;
    screening->seats_reserved = 0;
    screening->can_change = 1;

    return screening;
}

//ajout d'un room
int cinema_add_room(Cinema* cinema, const char* name, int rows, int cols) {
    if(!cinema) return 0;
    Room* room = room_create(name, rows, cols);
    if (!room) return 0;
    cinema->rooms[cinema->num_rooms++] = room;
    return 1;
}

//ajout d'un movie
int cinema_add_movie(Cinema* cinema, const char* title, int duration, AgeRating rating, char* genre) {
    if(!cinema) return 0;
    Movie* movie = movie_create(cinema->num_movies, title, duration, rating, genre);
    if (!movie) return 0;
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 1));
    cinema->movies[cinema->num_movies++] = movie;
    return 1;
}

//ajout d'un screening
int cinema_add_screening(Cinema* cinema, int movie_id, int room_id, time_t start_time, float price) {
    if(!cinema) return 0;
   
    Movie* movie = NULL;
    Room* room = NULL;
    // Recherche du film et de la salle par leurs ID
    for (int i = 0; i < cinema->num_movies; i++) {
        if (cinema->movies[i]->id == movie_id) {
            movie = cinema->movies[i];
            break;
        }
    }
    for (int j = 0; j < cinema->num_rooms; j++) {
        if (cinema->rooms[j]->id == room_id) {
            room = cinema->rooms[j];
            break;
        }
    }
    if (!movie || !room) return 0;

    Screening* screening = screening_create(movie, room, start_time, price);
    if (!screening) return 0;

    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 1));
    cinema->screenings[cinema->num_screenings++] = screening;
    return 1;
}

//delete de room
int remove_room(Cinema* cinema, int room_id) {
    if (!cinema) return 0;

    for (int i = 0; i < cinema->num_rooms; i++) {
        if (cinema->rooms[i]->id == room_id) {
            // Libération de la mémoire des sièges
            for (int j = 0; j < cinema->rooms[i]->capacity; j++) {
                free(cinema->rooms[i]->seats[j]);
            }
            free(cinema->rooms[i]->seats);
            free(cinema->rooms[i]);

            // Décalage des salles restantes
            for (int k = i; k < cinema->num_rooms - 1; k++) {
                cinema->rooms[k] = cinema->rooms[k + 1];
            }
            cinema->num_rooms--;
            return 1;
        }
    }
    return 0;
}
//delete de movie
int remove_movie(Cinema* cinema, int movie_id) {
    if (!cinema) return 0;

    for (int i = 0; i < cinema->num_movies; i++) {
        if (cinema->movies[i]->id == movie_id) {
            free(cinema->movies[i]);

            // Décalage des films restants
            for (int k = i; k < cinema->num_movies - 1; k++) {
                cinema->movies[k] = cinema->movies[k + 1];
            }
            cinema->num_movies--;
            return 1;
        }
    }
    return 0;
}

//delete de screening
int remove_screening(Cinema* cinema, int screening_id) {
    if (!cinema) return 0;

    for (int i = 0; i < cinema->num_screenings; i++) {
        if (cinema->screenings[i]->id == screening_id) {
            free(cinema->screenings[i]);

            // Décalage des séances restantes
            for (int k = i; k < cinema->num_screenings - 1; k++) {
                cinema->screenings[k] = cinema->screenings[k + 1];
            }
            cinema->num_screenings--;
            return 1;
        }
    }
    return 0;
}

//affichage des rooms
Room** list_rooms(Cinema* cinema, int* count)
{
    if (!cinema || cinema->num_rooms == 0) {
        *count = 0;
        return NULL;
    }

    *count = cinema->num_rooms;
    return cinema->rooms;
}

//affichage des movies
Movie** list_movies(Cinema* cinema, int* count)
{
    if (!cinema || cinema->num_movies == 0) {
        *count = 0;
        return NULL;
    }

    *count = cinema->num_movies;
    return cinema->movies;
}

//affichage des screenings
Screening** list_screenings(Cinema* cinema, int* count)
{
    if (!cinema || cinema->num_screenings == 0) {
        *count = 0;
        return NULL;
    }

    *count = cinema->num_screenings;
    return cinema->screenings;
}

//taux de remplissage d'un room
float calculate_occupancy(Screening* screening) {
    if (!screening || !screening->room) return 0.0f;

    int used = screening->seats_sold + screening->seats_reserved;
    return (float)used / screening->room->capacity;
}
//ticketList creation
TicketList* ticketlist_create() {
    TicketList* list = (TicketList*)malloc(sizeof(TicketList));
    if (!list) return NULL;

    list->head = NULL;
    list->actual = NULL;
    list->tail = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL); 

    return list;
}

//ticketList destruction
void ticketlist_destroy(TicketList* list) {
    if (!list) return;

    pthread_mutex_lock(&list->mutex);
    TicketNode* current = list->head;
    while (current) {
        TicketNode* temp = current;
        current = current->next;
        free(temp->ticket);
        free(temp);
    }
    pthread_mutex_unlock(&list->mutex);
    pthread_mutex_destroy(&list->mutex);
    free(list);
}
//gestion de la projection flexible
// 1. check du seuil
int check_max_capacity(Screening* screening) {
   return calculate_occupancy(screening) >= 0.9f;
}

//2. Bloquer le screening
int lock_screening(Screening* screening) {
    if (!screening) return 0;
    screening->can_change = 0;
    return 1;
}
int unlock_screening(Screening* screening) {
    if (!screening) return 0;
    screening->can_change = 1;
    return 1;
}

//3. notif admin
void notify_admin_threshold(Screening* screening)
{
    printf("[ADMIN] Séance %d presque pleine (%.2f%%)\n",
        screening->id,
        calculate_occupancy(screening) * 100);    
}

//4. changement de film
int can_switch_film(Screening* screening)
{
    if (!screening) return 0;

    float occupancy = calculate_occupancy(screening);
    return occupancy < 0.2f && screening->can_change;
}
int switch_film(Screening* screening, Movie* new_movie)
{
    if (!can_switch_film(screening)) return 0;

    screening->movie = new_movie;
    return 1;
}
void update_dynamic_schedule(Cinema* cinema)
{
    if (!cinema) return;

    for (int i = 0; i < cinema->num_screenings; i++) {
        Screening* s = cinema->screenings[i];

        if (check_max_capacity(s))
            notify_admin_threshold(s);
    }
}