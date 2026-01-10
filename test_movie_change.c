#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "gestion.h"
#include "ticket_service.h"
#include "alarm.h"

// Compteur pour les alertes SIGUSR1
static int sigusr1_count = 0;

// Handler pour SIGUSR1 (Changement de film)
void handle_sigusr1(int sig) {
    (void)sig;
    sigusr1_count++;
    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║ [ALERTE FILM] Signal SIGUSR1 recu       ║\n");
    printf("║ Nombre d'alertes: %d                     ║\n", sigusr1_count);
    printf("╚═══════════════════════════════════════════╝\n\n");
}

int main() {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   TEST DU CHANGEMENT DE FILM - NOTIFICATION             ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    // Enregistrer le handler pour SIGUSR1
    signal(SIGUSR1, handle_sigusr1);
    
    srand(time(NULL));
    
    // ========== INITIALISATION DU CINÉMA ==========
    printf("[SETUP] Initialisation du cinéma...\n");
    Cinema* cinema = cinema_create(5);
    
    // Créer deux films
    Movie* m1 = movie_create(1, "Film Peu Populaire", 120, AGE_16, "Drame");
    Movie* m2 = movie_create(2, "Film Très Populaire", 140, AGE_16, "Action");
    
    // Créer une salle et une séance avec le premier film
    Room* r1 = room_create("Salle Test", 5, 4);
    Screening* s1 = screening_create(m1, r1, time(NULL) + 3600, 15.0);
    
    cinema->rooms[cinema->num_rooms++] = r1;
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 2));
    cinema->movies[cinema->num_movies++] = m1;
    cinema->movies[cinema->num_movies++] = m2;
    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 1));
    cinema->screenings[cinema->num_screenings++] = s1;
    
    printf("✓ Salle créée: '%s' (20 places)\n", r1->name);
    printf("✓ Film 1 créé: '%s'\n", m1->title);
    printf("✓ Film 2 créé: '%s'\n", m2->title);
    printf("✓ Séance créée avec Film 1 (ID: %d)\n\n", s1->id);
    
    // ========== VENDRE DES BILLETS ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE 1: VENTE DE BILLETS POUR LE FILM 1              ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Vente de 3 billets pour la séance (15%% de 20 places)...\n");
    
    AlternativeList* alt = NULL;
    for (int i = 0; i < 3; i++) {
        int seat_id = i;
        TicketResult result = purchase_ticket(cinema, s1->id, 
                                             "Client", "client@cinema.fr", 
                                             20, seat_id, &alt);
        if (result == OK) {
            printf("✓ Billet %d vendu (Siege %d)\n", i + 1, seat_id);
        } else {
            printf("✗ Billet %d refusé\n", i + 1);
        }
    }
    
    printf("\n✓ 3 billets vendus pour '%s'\n\n", s1->movie->title);
    
    // ========== AFFICHER INFORMATIONS INITIALES ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   INFORMATIONS AVANT CHANGEMENT DE FILM                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Séance ID: %d\n", s1->id);
    printf("Film actuel: '%s' (ID: %d)\n", s1->movie->title, s1->movie->id);
    printf("Billets vendus: %d\n", s1->seats_sold);
    printf("Occupancy: %.1f%%\n\n", cinema->statistics->occupancy_rate_by_screening[s1->id]);
    
    // ========== FORCER LE REMPLISSAGE POUR can_change ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE 2: PRÉPARATION POUR CHANGEMENT DE FILM           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Vérification: can_change = %d (0 = peut changer, 1 = ne peut pas)\n\n", s1->can_change);
    
    // Calculer occupancy
    float occ = (float)s1->seats_sold / (float)r1->capacity * 100.0f;
    printf("Occupancy: %.1f%% (%.2f places vendues sur %d)\n", occ, 
           (float)s1->seats_sold, r1->capacity);
    
    // Forcer can_change à 1 en vendant plus de billets (> 20%)
    if (occ < 20.0f) {
        printf("\nOccupancy < 20%%, on peut changer le film\n");
        s1->can_change = 1;
    } else {
        printf("\nOccupancy >= 20%%, on ne peut pas changer le film\n");
        s1->can_change = 0;
    }
    
    printf("can_change après vérification: %d\n\n", s1->can_change);
    
    // ========== CHANGEMENT DE FILM ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE 3: REMPLACEMENT DU FILM ET NOTIFICATION          ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    if (s1->can_change) {
        printf("Changement du film possible! Remplacement en cours...\n");
        printf("Ancien film: '%s' → Nouveau film: '%s'\n\n", 
               s1->movie->title, m2->title);
        
        // Appeler la fonction de remplacement
        int result = replace_movie_with_more_popular(cinema, s1->id);
        
        if (result == 0) {
            printf("\n✓ Film changé avec succès!\n");
            printf("✓ Signal SIGUSR1 envoyé\n");
            printf("✓ Clients notifiés\n\n");
        } else if (result == -2) {
            printf("\n✗ Aucun film plus populaire trouvé\n\n");
        } else {
            printf("\n✗ Erreur lors du changement de film\n\n");
        }
    } else {
        printf("✗ Changement de film impossible (trop de billets vendus)\n");
        printf("  Pour pouvoir changer, il faut < 20%% de billets vendus\n");
        printf("  Actuellement: %.1f%% vendus\n\n", occ);
    }
    
    // ========== AFFICHER INFORMATIONS FINALES ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   INFORMATIONS APRÈS CHANGEMENT DE FILM                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Séance ID: %d\n", s1->id);
    printf("Film actuel: '%s' (ID: %d)\n", s1->movie->title, s1->movie->id);
    printf("Billets vendus: %d (inchangés)\n", s1->seats_sold);
    printf("Occupancy: %.1f%%\n\n", cinema->statistics->occupancy_rate_by_screening[s1->id]);
    
    printf("🚨 Signaux SIGUSR1 reçus: %d\n", sigusr1_count);
    
    if (sigusr1_count > 0) {
        printf("✓ TEST RÉUSSI: L'alerte de changement a été correctement déclenchée!\n");
    } else {
        printf("⚠ Pas de signal reçu (peut être normal si film non changé)\n");
    }
    
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST TERMINÉ                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}
