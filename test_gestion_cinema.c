// on va tester les fonctions liees a la gestion des tickets
#include "gestion.h"

int main(void){
    printf("----- Cinema Management System Test -----\n");
    // Créer un cinéma avec une capacité pour 5 salles
    Cinema* cinema = cinema_create(5);
    if (!cinema) {
        printf("Échec de la création du cinéma.\n");
        return 1;
    }else printf("Cinema ID: %d created.\n", cinema->id);

    // Ajouter des salles
    char room_name[50];
    for (int i = 0; i < 4; i++) {
        snprintf(room_name, sizeof(room_name), "Salle %d", i+1);

        if (cinema_add_room(cinema, room_name, 10, 15)) {
            printf("Room '%s' added.\n", room_name);
        } else { 
            printf("Échec de l'ajout de la salle.\n");
            return 1;
        } 
    } 

    //Ajout des films
    if (!cinema_add_movie(cinema, "Inception", 148, AGE_12, "Science Fiction")) {
        printf("Échec de l'ajout du film 'Inception'.\n");
        return 1;
    }else printf("Movie 'Inception' added.\n");

    if (!cinema_add_movie(cinema, "The Dark Knight", 152, AGE_16, "Action")) {
        printf("Échec de l'ajout du film 'The Dark Knight'.\n");
        return 1;
    }else printf("Movie 'The Dark Knight' added.\n");

    if (!cinema_add_movie(cinema, "Interstellar", 169, AGE_18, "Science Fiction")) {
        printf("Échec de l'ajout du film 'Interstellar'.\n");
        return 1;
    }else printf("Movie 'Interstellar' added.\n");

    if (!cinema_add_movie(cinema, "La La Land", 128, AGE_ALL, "Musical")) {
        printf("Échec de l'ajout du film 'La La Land'.\n");
        return 1;
    }else printf("Movie 'La La Land' added.\n");

    // Ajouter des séances
    time_t start_time = time(NULL) + 3600; // séance dans une heure
    if (!cinema_add_screening(cinema, 0, 0, start_time, 10.0f)) {
        printf("Échec de l'ajout de la séance.\n");
        return 1;
    }else printf("Screening for movie ID 0 in room ID 0 added.\n");

    if (!cinema_add_screening(cinema, 2, 1, start_time, 10.0f)) {
        printf("Échec de l'ajout de la séance.\n");
        return 1;
    }else printf("Screening for movie ID 2 in room ID 1 added.\n");

    if (!cinema_add_screening(cinema, 1, 2, start_time, 10.0f)) {
        printf("Échec de l'ajout de la séance.\n");
        return 1;
    }else printf("Screening for movie ID 1 in room ID 2 added.\n");

    if (!cinema_add_screening(cinema, 3, 3, start_time, 10.0f)) {
        printf("Échec de l'ajout de la séance.\n");
        return 1;
    }else printf("Screening for movie ID 3 in room ID 3 added.\n");

    // Lister les salles
    int room_count = 0;
    Room** rooms = list_rooms(cinema, &room_count);
    printf("Nombre de salles: %d\n", room_count);
    for (int i = 0; i < room_count; i++) {
        printf("Salle ID: %d, Nom: %s, Capacité: %d\n", rooms[i]->id, rooms[i]->name, rooms[i]->capacity);
    }
    // Lister les films
    int movie_count = 0;
    Movie** movies = list_movies(cinema, &movie_count);
    printf("Nombre de films: %d\n", movie_count);
    for (int i = 0; i < movie_count; i++) {
        printf("Film ID: %d, Titre: %s, Durée: %d minutes, Genre: %s\n", movies[i]->id, movies[i]->title, movies[i]->duration_minutes, movies[i]->genre);
    }
    // Lister les séances
    int screening_count = 0;
    Screening** screenings = list_screenings(cinema, &screening_count);
    printf("Nombre de séances: %d\n", screening_count);
    for (int i = 0; i < screening_count; i++) {
        printf("Séance ID: %d, Film: %s, Salle: %s, Heure de début: %s, Occupation %.2f%%\n", screenings[i]->id, screenings[i]->movie->title, screenings[i]->room->name, ctime(&screenings[i]->start_time),calculate_occupancy(screenings[i]));
    }

    //Test des fonctions de gestion dynamique
    printf("\n--- Testing Dynamic Screening Management ---\n");
    Screening* test_screening = screenings[0];
    printf("Initial occupancy for Screening ID %d: %.2f%%\n", test_screening->id, calculate_occupancy(test_screening) * 100);
    if (can_switch_film(test_screening)) {
        printf("Can switch film for Screening ID %d.\n", test_screening->id);
    } else {
        printf("Cannot switch film for Screening ID %d.\n", test_screening->id);
    }
    // Simuler la vente de billets pour augmenter l'occupation
    test_screening->seats_sold = (int)(test_screening->room->capacity * 0.15); // 15% des sièges vendus
    printf("Updated occupancy for Screening ID %d: %.2f%%\n", test_screening->id, calculate_occupancy(test_screening) * 100);
    if (can_switch_film(test_screening)) {
        printf("Can switch film for Screening ID %d.\n", test_screening->id);
    } else {
        printf("Cannot switch film for Screening ID %d.\n", test_screening->id);
    }
    // Vendre plus de billets pour dépasser le seuil de 90%
    test_screening->seats_sold = (int)(test_screening->room->capacity * 0.92); // 92% des sièges vendus
    printf("Final occupancy for Screening ID %d: %.2f%%\n", test_screening->id, calculate_occupancy(test_screening) * 100);
    update_dynamic_schedule(cinema);
    //switch de film
    Movie* new_movie = movies[3]; // La La Land
    if (switch_film(test_screening, new_movie)) {
        printf("Film switched to '%s' for Screening ID %d.\n", new_movie->title, test_screening->id);
    } else {
        printf("Failed to switch film for Screening ID %d.\n", test_screening->id);
    }

    // Nettoyage
    printf("\n Nettoyage et libération de la mémoire.\n");
    //suppression des screenings
    for (int i = screening_count - 1; i >= 0; i--) {
        remove_screening(cinema, screenings[i]->id);
        printf("Ok\n");
    }
    //suppression des movies
    for (int i = movie_count - 1; i >= 0; i--){
        remove_movie(cinema, movies[i]->id);
        printf("OK\n");
    }
    //suppression des rooms
    for (int i = room_count - 1; i >= 0; i--){
        remove_room(cinema, rooms[i]->id);
        printf("OK.\n");
    }
    //liberation du cinema 
    free(cinema);

    return 0;
}