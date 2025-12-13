typedef enum { SEAT_AVAILABLE, SEAT_RESERVED, SEAT_SOLD } SeatStatus;

typedef enum { TICKET_VALID, TICKET_USED, TICKET_CANCELLED, TICKET_EXCHANGED } TicketStatus;

typedef enum { AGE_ALL, // Tous publics AGE_12, // -12 ans AGE_16, // -16 ans AGE_18 // -18 ans } AgeRating;

typedef struct { int id; int row; int col; SeatStatus status; int ticket_id; // -1 si libre } Seat;

typedef struct { int id; char title[100]; int duration_minutes; AgeRating age_rating; char genre[50]; } Movie;

typedef struct { int id; char name[50]; int capacity; Seat* seats; // Tableau dynamique int rows; int cols; int available_seats; } Room;

typedef struct { int id; Movie* movie; Room* room; time_t start_time; float price; int seats_sold; int seats_reserved; int can_change; // 1 si < 20% vendus } Screening;

typedef struct { int id; char customer_name[100]; char email[100]; int age; Screening* screening; int seat_id; TicketStatus status; time_t purchase_time; time_t reservation_time; // 0 si achat direct int is_reservation; } Ticket;

typedef struct TicketNode { Ticket* ticket; struct TicketNode* next; } TicketNode;

typedef struct { TicketNode* head; TicketNode* tail; int size; pthread_mutex_t mutex; // Sécurité thread } TicketQueue;

typedef struct { int id; Screening** screenings; // Tableau dynamique int num_screenings; Room** rooms; int num_rooms; Movie** movies; int num_movies; Ticket** tickets; int num_tickets; TicketQueue* kiosk_queue; // File guichet auto TicketQueue* counter_queue; // File hôtesse } Cinema;
