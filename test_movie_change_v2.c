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
    printf("║   TEST: CHANGEMENT DE FILM + NOTIFICATION CLIENTS        ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    // Enregistrer le handler pour SIGUSR1
    signal(SIGUSR1, handle_sigusr1);
    
    srand(time(NULL));
    
    // ========== INITIALISATION DU CINÉMA ==========
    printf("[SETUP] Initialisation du cinéma...\n");
    Cinema* cinema = cinema_create(5);
    
    // Créer 3 films de catégories différentes
    Movie* m1 = movie_create(1, "Film Peu Populaire", 120, AGE_16, "Drame");
    Movie* m2 = movie_create(2, "Film Très Populaire", 140, AGE_16, "Action");
    Movie* m3 = movie_create(3, "Film Super Populaire", 130, AGE_16, "Aventure");
    
    // Créer deux salles et plusieurs séances
    Room* r1 = room_create("Salle 1", 5, 4);
    Room* r2 = room_create("Salle 2", 5, 4);
    
    // Séance 1: Film peu populaire (sera changée)
    Screening* s1 = screening_create(m1, r1, time(NULL) + 3600, 15.0);
    
    // Séance 2: Film très populaire (sera la référence pour le changement)
    Screening* s2 = screening_create(m2, r2, time(NULL) + 3600, 15.0);
    
    cinema->rooms[cinema->num_rooms++] = r1;
    cinema->rooms[cinema->num_rooms++] = r2;
    
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 3));
    cinema->movies[cinema->num_movies++] = m1;
    cinema->movies[cinema->num_movies++] = m2;
    cinema->movies[cinema->num_movies++] = m3;
    
    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 2));
    cinema->screenings[cinema->num_screenings++] = s1;
    cinema->screenings[cinema->num_screenings++] = s2;
    
    printf("✓ 3 films créés\n");
    printf("✓ 2 salles créées\n");
    printf("✓ 2 séances créées\n\n");
    
    // ========== VENDRE DES BILLETS ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE 1: VENTE DE BILLETS POUR LES DEUX SÉANCES        ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    AlternativeList* alt = NULL;
    
    // Vendre 3 billets pour la séance 1 (15%)
    printf("Vente pour Séance 1 (Film Peu Populaire) - 3 billets:\n");
    for (int i = 0; i < 3; i++) {
        TicketResult result = purchase_ticket(cinema, s1->id, 
                                             "Client", "client@cinema.fr", 
                                             20, i, &alt);
        if (result == OK) {
            printf("  ✓ Billet vendu\n");
        }
    }
    
    // Vendre 12 billets pour la séance 2 (60%)
    printf("\nVente pour Séance 2 (Film Très Populaire) - 12 billets:\n");
    for (int i = 0; i < 12; i++) {
        TicketResult result = purchase_ticket(cinema, s2->id, 
                                             "Client", "client@cinema.fr", 
                                             20, i, &alt);
        if (result == OK && i % 3 == 0) {
            printf("  ✓ Billets vendus\n");
        }
    }
    printf("\n");
    
    // ========== AFFICHER INFORMATIONS ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   INFORMATIONS AVANT CHANGEMENT                          ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("SÉANCE 1:\n");
    printf("  Film: '%s'\n", s1->movie->title);
    printf("  Billets vendus: %d/20 (%.1f%%)\n", s1->seats_sold, 
           cinema->statistics->occupancy_rate_by_screening[s1->id]);
    printf("  Can_change: %d (1=peut changer)\n", s1->can_change);
    
    printf("\nSÉANCE 2:\n");
    printf("  Film: '%s'\n", s2->movie->title);
    printf("  Billets vendus: %d/20 (%.1f%%)\n", s2->seats_sold, 
           cinema->statistics->occupancy_rate_by_screening[s2->id]);
    printf("  Can_change: %d\n\n", s2->can_change);
    
    // ========== CHANGEMENT DE FILM ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE 2: CHANGEMENT DE FILM (Séance 1)                 ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    if (s1->can_change) {
        printf("✓ Changement possible pour Séance 1!\n");
        printf("  Ancien film: '%s'\n", s1->movie->title);
        
        int result = replace_movie_with_more_popular(cinema, s1->id);
        
        if (result == 0) {
            printf("  Nouveau film: '%s'\n\n", s1->movie->title);
            printf("✓ Film changé avec succès!\n");
            printf("✓ Signal SIGUSR1 envoyé\n");
            printf("✓ Clients notifiés\n\n");
        } else if (result == -2) {
            printf("\n⚠ Aucun film plus populaire trouvé\n");
            printf("  (Il faut une séance avec le même AGE_RATING et meilleure occupancy)\n\n");
        } else {
            printf("\n✗ Erreur lors du changement (code: %d)\n\n", result);
        }
    } else {
        printf("✗ Changement impossible (trop de billets vendus)\n\n");
    }
    
    // ========== AFFICHER INFORMATIONS FINALES ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   INFORMATIONS APRÈS CHANGEMENT                          ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("SÉANCE 1:\n");
    printf("  Film: '%s' (changé de '%s')\n", s1->movie->title, m1->title);
    printf("  Billets vendus: %d/20 (inchangé)\n", s1->seats_sold);
    printf("  Occupancy: %.1f%%\n\n", cinema->statistics->occupancy_rate_by_screening[s1->id]);
    
    printf("📊 Résumé des notifications:\n");
    printf("  Billets vendus affectés: %d\n", s1->seats_sold);
    printf("  Signaux SIGUSR1 reçus: %d\n\n", sigusr1_count);
    
    if (sigusr1_count > 0) {
        printf("✓ TEST RÉUSSI: Notification correctement déclenchée!\n");
    } else if (s1->movie->id == m1->id) {
        printf("⚠ Film non changé (pas de film plus populaire avec la même catégorie d'âge)\n");
    }
    
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST TERMINÉ                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}
