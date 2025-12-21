#include "gestion.h"

//Fonction de creation du cinema
static volatile int id_cinema=0;
static volatile int id_room=0;
static volatile int id_screening=0;
static volatile int id_ticket=0;


int initialisation()
{
return 0;
}



Cinema* cinema_create(int num_rooms_total) {
    Cinema* cinema = (Cinema*)malloc(sizeof(Cinema));
    if (!cinema) return NULL;

    cinema->id = id_cinema++;
    cinema->num_rooms = 0;
    cinema->rooms = (Room**)calloc(num_rooms_total, sizeof(Room*));
    cinema->num_movies = 0;
    cinema->movies = NULL;
    cinema->num_screenings = 0;
    cinema->screenings = NULL;
    cinema->num_tickets = 0;
    cinema->tickets = NULL;
    cinema->statistics->total_tickets_sold = 0;
    cinema->statistics->total_tickets_reserved = 0;
    cinema->statistics->total_tickets_cancelled = 0;
    cinema->statistics->total_ticket_exchanged = 0;
    cinema->statistics->total_revenue = 0.0;
    cinema->statistics->avg_waiting_time_counter = 0.0;
    cinema->statistics->avg_waiting_time_kiosk = 0.0;
    for (int i=0; i<50;i++)
    {
        cinema->statistics->occupancy_rate[i] = 0;
        cinema->statistics->tickets_by_room[i] = 0;
    }
    for (int i=0; i<100;i++)
    {
        cinema->statistics->tickets_by_movie[i] = 0;
    }


    // Initialiser les listes
    cinema->kiosk_list = ticketlist_create();
    cinema->counter_list = ticketlist_create();

    return cinema;
}

Room* room_create(const char* name, int rows, int cols) {
    Room* room = (Room*)malloc(sizeof(Room));
    if (!room) return NULL;

    room->id = id_room++;
    strncpy(room->name, name, sizeof(room->name) - 1);
    room->rows = rows;
    room->cols = cols;
    room->capacity = rows * cols;
    room->available_seats = room->capacity;
    room->isreserveforevent=0;

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

Screening* screening_create(Movie* movie, Room* room, time_t start_time, float price)
{
    Screening* screening = (Screening*)malloc(sizeof(Screening));
    if (!screening) return NULL;

    screening->id = id_screening++;
    screening->movie = movie;
    screening->room = room;
    // date du film
    screening->start_time = start_time;
    screening->price = price;
    screening->seats_sold = 0;
    screening->seats_reserved = 0;
    screening->can_change = 1;

    return screening;
}

/* Ticket* ticket_create(char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t time, int is_reservation)
{
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    if (!ticket) return NULL;

    ticket->id = id_ticket++;
    strncpy(ticket->customer_name, customer_name, sizeof(ticket->customer_name) - 1);
    strncpy(ticket->email, email, sizeof(ticket->email) - 1);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    if (is_reservation)
    {
        ticket->reservation_time = time;
        ticket->screening->seats_reserved++;
        ticket->screening->room->available_seats--;
    }
    ticket->purchase_time = time;
    ticket->is_reservation = is_reservation;

    return ticket;
} */


Ticket* achat_ticket(Cinema* cinema, char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t buyingtime)
{
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    if (!ticket) return NULL;

    ticket->id = id_ticket++;
    strncpy(ticket->customer_name, customer_name, sizeof(ticket->customer_name) - 1);
    strncpy(ticket->email, email, sizeof(ticket->email) - 1);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    ticket->reservation_time =0;
    ticket->purchase_time = buyingtime;
    ticket->is_reservation =0;
    ticket->screening->room->seats[seat_id].status = SEAT_SOLD;
    ticket->screening->room->seats[seat_id].ticket_id = ticket->id;
    ticket->screening->room->available_seats--;
    ticket->screening->seats_sold++;
    cinema->num_tickets++;
    cinema->tickets = realloc(cinema->tickets, sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = ticket;
    cinema->statistics->total_tickets_sold++;
    cinema->statistics->tickets_by_movie[ticket->screening->movie->id]++;
    cinema->statistics->tickets_by_room[ticket->screening->room->id]++;
    cinema->statistics->total_revenue+=ticket->screening->price;
    return ticket;
}

//fonction pour la reservation des billets
Ticket* reservation_ticket(Cinema* cinema, char* customer_name, char* email, int age, Screening* screening, int seat_id,time_t reservationtime)
{
    Ticket* ticket = (Ticket*)malloc(sizeof(Ticket));
    if (!ticket) return NULL;

    ticket->id = id_ticket++;
    strncpy(ticket->customer_name, customer_name, sizeof(ticket->customer_name) - 1);
    strncpy(ticket->email, email, sizeof(ticket->email) - 1);
    ticket->age = age;
    ticket->screening = screening;
    ticket->seat_id = seat_id;
    ticket->status = TICKET_VALID;
    ticket->reservation_time =reservationtime;
    ticket->purchase_time = 0;
    ticket->is_reservation =1;
    ticket->screening->room->seats[seat_id].status = SEAT_RESERVED;
    ticket->screening->room->seats[seat_id].ticket_id = ticket->id;
    ticket->screening->room->available_seats--;
    ticket->screening->seats_reserved++;
    cinema->num_tickets++;
    cinema->tickets = realloc(cinema->tickets, sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = ticket;
    cinema->statistics->total_tickets_reserved++;
    return ticket;
}

//fonction pour la gestion de l'annulation d'une reservation
int annulation_ticket(Cinema* cinema, Ticket* ticket, TicketStatus status)
{
    ticket->status = TICKET_CANCELLED;
    ticket->screening->room->seats[ticket->seat_id].status = SEAT_AVAILABLE;
    ticket->screening->room->seats[ticket->seat_id].ticket_id = -1;
    ticket->screening->room->available_seats++;
    ticket->screening->seats_reserved--;
    cinema->statistics->total_tickets_sold++;
    cinema->statistics->tickets_by_movie[ticket->screening->movie->id]++;
    cinema->statistics->tickets_by_room[ticket->screening->room->id]++;
    cinema->statistics->total_revenue+=ticket->screening->price;

    return 1;
}

//fonction permettant l'echange de billet
Ticket* echange_billet(Cinema* cinema, Ticket* ticket, Screening* screening, int seat_id, time_t exchangingtime)
{
    Ticket* new_ticket = (Ticket*)malloc(sizeof(Ticket));
    if (!new_ticket) return NULL;

    new_ticket->id = id_ticket++;
    strncpy(new_ticket->customer_name, ticket->customer_name, sizeof(new_ticket->customer_name) - 1);
    strncpy(new_ticket->email, ticket->email, sizeof(new_ticket->email) - 1);
    new_ticket->age = ticket->age;
    new_ticket->screening = screening;
    new_ticket->seat_id = seat_id;
    new_ticket->status = TICKET_VALID;
    new_ticket->reservation_time =0;
    new_ticket->purchase_time = exchangingtime;
    new_ticket->is_reservation =0;
    new_ticket->screening->room->seats[seat_id].status = SEAT_SOLD;
    new_ticket->screening->room->seats[seat_id].ticket_id = new_ticket->id;
    new_ticket->screening->room->available_seats--;
    new_ticket->screening->seats_sold++;

    ticket->status = TICKET_EXCHANGED;
    ticket->screening->room->seats[seat_id].status = SEAT_AVAILABLE;
    ticket->screening->room->seats[seat_id].ticket_id = -1;
    ticket->screening->room->available_seats++;
    ticket->screening->seats_sold--;
    cinema->num_tickets++;
    cinema->tickets = realloc(cinema->tickets, sizeof(Ticket*) * cinema->num_tickets);
    cinema->tickets[cinema->num_tickets - 1] = new_ticket;

    return new_ticket;
}

//fonction pour la gestion de la libération de place apres un visionnage
int liberation_places(Screening* screening)
{

    return 0;
}

//retourne la disponibilite d'une salle
int is_roomavailable(Room* room)
{
    if (room->isreserveforevent==1)
    {
        return 0;
    }
    return 1;
}

//retourne la disponibilite d'une place et donc de la salle ou elle se trouve
int is_seatavailable(Screening* screening, int id_seat)
{
    if (screening->room->seats[id_seat].status==SEAT_AVAILABLE && is_roomavailable(screening->room))
    {
        return 1;
    }
    return 0;
}

//renvoi le statut de remplissage d'une salle
int is_roomfull(Room* room)
{
    if (room->available_seats < room->capacity*0.9)
    {
        return 0;
    }
    return 1;
}

CinemaStatistics* generate_cinemastatistics(Cinema* cinema)
{
    CinemaStatistics* statistics = (CinemaStatistics*)malloc(sizeof(CinemaStatistics));
    if (!statistics) return NULL;

    statistics->total_tickets_sold = cinema;
    int total_tickets_reserved;
    int total_tickets_cancelled;
    float total_revenue;
    float occupancy_rate;
    int tickets_by_movie[100];  // Index = movie_id
    int tickets_by_room[50];    // Index = room_id
    float avg_waiting_time_kiosk;
    float avg_waiting_time_counter;
    int exchanges_count;
}
