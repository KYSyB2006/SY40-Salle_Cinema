#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "gestion.h"
#include "threads.h"
#include "ticket_service.h"
#include "reservation_service.h"
#include "alternatives.h"


int main() {
	srand(time(NULL));

	Cinema* cinema = cinema_create(5);

	// Création film / salle / séance en utilisant les fonctions de gestion
	Room* r1 = room_create("Salle 1", 5, 5);
	Movie* m1 = movie_create(1, "Matrix", 120, AGE_16, "Action");
	Screening* s1 = screening_create(m1, r1, time(NULL) + 3600, 10.0);

	// Ajout manuel dans les tableaux (cinema_create a alloué l'espace pour rooms)
	cinema->rooms[cinema->num_rooms++] = r1;
	cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 1));
	cinema->movies[cinema->num_movies++] = m1;
	cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 1));
	cinema->screenings[cinema->num_screenings++] = s1;

	// Ajouter une deuxième salle, film et séance
	Room* r2 = room_create("Salle 2", 4, 4);
	Movie* m2 = movie_create(2, "Inception", 148, AGE_12, "Sci-Fi");
	Screening* s2 = screening_create(m2, r2, time(NULL) + 5400, 9.0);

	cinema->rooms[cinema->num_rooms++] = r2;
	cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 1));
	cinema->movies[cinema->num_movies++] = m2;
	cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 1));
	cinema->screenings[cinema->num_screenings++] = s2;

	// client_queue déjà initialisée par cinema_create

	// Threads agents
	pthread_t hostess[2], kiosk[2], processor;

	pthread_create(&processor, NULL, processor_thread, cinema);

	for (int i = 0; i < 2; i++) {
		pthread_create(&hostess[i], NULL, hostess_thread, cinema);
		pthread_create(&kiosk[i], NULL, kiosk_thread, cinema);
	}

	// Clients simulés
	pthread_t clients[6];
	for (int i = 0; i < 6; i++) {
		pthread_create(&clients[i], NULL, client_thread, cinema);
	}

    pthread_t clients2[4];
	for (int i = 0; i < 4; i++) {
		pthread_create(&clients2[i], NULL, client_thread2, cinema);
	}

	// Join clients
	for (int i = 0; i < 6; i++) {
		pthread_join(clients[i], NULL);
	}
    for (int i = 0; i < 4; i++) {
		pthread_join(clients2[i], NULL);
	}

	// Laisser tourner le processeur/hotesse un peu pour traiter les intentions
	sleep(20);

    printf("Billets vendus : %d\n", cinema->statistics->total_tickets_sold);
    printf("Echanges : %d\n", cinema->statistics->total_ticket_exchanged);
    printf("Annulation : %d\n", cinema->statistics->total_tickets_cancelled);
    printf("Rembourses : %d\n", cinema->statistics->total_ticket_refunded);
    printf("Revenu total : %.2f\n", cinema->statistics->total_revenue);
    printf("Rservations : %d\n", cinema->statistics->total_tickets_reserved);
	printf("Rservations validees : %d\n", cinema->statistics->total_rsv_validated);
	printf("Rservations modifiees : %d\n", cinema->statistics->total_rsv_modified);
	printf("Rservations annulees : %d\n", cinema->statistics->total_rsv_cancelled);
	printf("alternatives gen : %d\n", cinema->statistics->alternatives_generated);
	printf("alternatives used : %d\n", cinema->statistics->alternatives_used);

	printf("=== FIN SIMULATION ===\n");
	return 0;
}
