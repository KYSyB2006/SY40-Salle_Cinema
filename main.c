// Gestion et simulation globale du cinéma
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "gestion.h"
#include "threads.h"
#include "ticket_service.h"
#include "reservation_service.h"
#include "alternatives.h"
#include "alarm.h"
#include "handle.h"

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand(time(NULL));
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    printf("\n===== CINEMA GLOBAL SIMULATION START =====\n");

    /* ===== Création du cinéma ===== */
    Cinema* cinema = cinema_create(6);

    /* ===== Rooms ===== */
    cinema_add_room(cinema, "Salle A", 8, 10);
    cinema_add_room(cinema, "Salle B", 6, 8);
    cinema_add_room(cinema, "Salle C", 10, 12);

    /* ===== Movies ===== */
    cinema_add_movie(cinema, "Matrix", 120, AGE_16, "Action");
    cinema_add_movie(cinema, "Inception", 148, AGE_12, "Sci-Fi");
    cinema_add_movie(cinema, "Interstellar", 169, AGE_18, "Sci-Fi");
    cinema_add_movie(cinema, "La La Land", 128, AGE_ALL, "Musical");

    /* ===== Screenings ===== */
    time_t now = time(NULL);

    // Séance courte → scheduler
    cinema_add_screening(cinema, 0, 0, now - 120*60, 10.0);

    // Séance classique
    cinema_add_screening(cinema, 1, 1, now + 20, 9.0);

    // Séance pour switch film
    cinema_add_screening(cinema, 2, 2, now + 30, 11.0);

    //
    cinema_add_screening(cinema, 3, 0, now + 100, 11.0);

    printf("[INIT] Cinema ready (%d rooms | %d movies | %d screenings)\n",
           cinema->num_rooms, cinema->num_movies, cinema->num_screenings);

    /* ===== PRÉ-REMPLISSAGE : TICKETS RÉELS ===== */
    printf("\n[BOOTSTRAP] Creating initial tickets...\n");

    for (int i = 0; i < 3; i++) {
        purchase_ticket(
            cinema,
            cinema->screenings[3]->id,
            "BOOT_TICKET",
            "boot@mail.com",
            25,
            i,
            NULL
        );
    }

    /* ===== PRÉ-REMPLISSAGE : RÉSERVATIONS ===== */
    printf("[BOOTSTRAP] Creating initial reservations...\n");

    for (int i = 0; i < 5; i++) {
        make_reservation(
            cinema,
            cinema->screenings[1]->id,
            "BOOT_RSV",
            "rsv@mail.com",
            30,
            i,
            NULL
        );
    }

    /* ===== FORCER ALERTE 90% ===== */
    Screening* s_alert = cinema->screenings[2];
    s_alert->seats_sold = (int)(s_alert->room->capacity * 0.92);
    s_alert->can_change = 1;
    
    printf("[FORCE] Screening %d forced to %.2f%% occupancy\n",
           s_alert->id, calculate_occupancy(s_alert) * 100);

    /* ===== Threads ===== */
    pthread_t processor, supervisor, scheduler;
    pthread_t hostess[2], kiosk[2];
    pthread_t clients[8], clients_rsv[5];

    pthread_create(&processor, NULL, processor_thread, cinema);
    pthread_create(&supervisor, NULL, supervisor_thread, cinema);
    pthread_create(&scheduler, NULL, scheduler_thread, cinema);


    for (int i = 0; i < 2; i++) {
        pthread_create(&hostess[i], NULL, hostess_thread, cinema);
        pthread_create(&kiosk[i], NULL, kiosk_thread, cinema);
    }

    /* ===== Clients simulés ===== */
    for (int i = 0; i < 8; i++)
        pthread_create(&clients[i], NULL, client_thread, cinema);

    for (int i = 0; i < 5; i++)
        pthread_create(&clients_rsv[i], NULL, client_thread2, cinema);

    /* ===== Attente ===== */
    for (int i = 0; i < 8; i++) pthread_join(clients[i], NULL);
    for (int i = 0; i < 5; i++) pthread_join(clients_rsv[i], NULL);

    printf("\n[MAIN] Clients finished — simulation running...\n");

    sleep(20);

    /* ===== STATISTIQUES ===== */
    CinemaStatistics* s = cinema->statistics;

    printf("\n===== FINAL STATISTICS =====\n");
    printf("Tickets sold          : %d\n", s->total_tickets_sold);
    printf("Tickets exchanged     : %d\n", s->total_ticket_exchanged);
    printf("Tickets refunded      : %d\n", s->total_ticket_refunded);
    printf("Tickets cancelled     : %d\n", s->total_tickets_cancelled);
    printf("Reservations made     : %d\n", s->total_tickets_reserved);
    printf("Reservations validated: %d\n", s->total_rsv_validated);
    printf("Reservations modified : %d\n", s->total_rsv_modified);
    printf("Reservations cancelled : %d\n", s->total_rsv_cancelled);
    printf("Alternatives generated: %d\n", s->alternatives_generated);
    printf("Alternatives used     : %d\n", s->alternatives_used);
    printf("Total revenue         : %.2f\n", s->total_revenue);

    printf("\n===== SIMULATION END =====\n");
    return 0;
}
