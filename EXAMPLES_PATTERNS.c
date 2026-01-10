#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "alarm.h"
#include "gestion.h"

/*
 * EXEMPLE: Comment intégrer le CapacityMonitor dans votre application
 * 
 * Ce fichier montre tous les patterns d'utilisation possibles
 */

// ============================================================
// 1. HANDLER DU SIGNAL SIGUSR2
// ============================================================

void my_sigusr2_handler(int sig) {
    (void)sig;
    printf("\n[APP] Une séance a atteint 90%% de capacité!\n");
    printf("[APP] Vous pouvez déclencher des actions spéciales ici.\n\n");
}

// ============================================================
// 2. PATTERN BASIQUE: Initialisation et arrêt
// ============================================================

void pattern_basic_usage() {
    printf("\n=== PATTERN 1: Utilisation basique ===\n");
    
    Cinema* cinema = cinema_create(10);
    
    // Initialiser le moniteur
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    
    // Démarrer le monitoring (vérification chaque 1000ms)
    capacity_monitor_start(monitor, 1000);
    
    // ... Votre code ici ...
    
    // Arrêter le monitoring
    capacity_monitor_stop(monitor);
    
    // Libérer les ressources
    capacity_monitor_destroy(monitor);
}

// ============================================================
// 3. PATTERN: Avec gestion du signal
// ============================================================

void pattern_with_signal_handler() {
    printf("\n=== PATTERN 2: Avec handler de signal ===\n");
    
    // Enregistrer le handler
    signal(SIGUSR2, my_sigusr2_handler);
    
    Cinema* cinema = cinema_create(10);
    
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    capacity_monitor_start(monitor, 1000);
    
    // ... Votre code ici ...
    // À chaque fois que 90% est atteint, my_sigusr2_handler() sera appelé
    
    capacity_monitor_stop(monitor);
    capacity_monitor_destroy(monitor);
}

// ============================================================
// 4. PATTERN: Intervalle de vérification court
// ============================================================

void pattern_fast_checking() {
    printf("\n=== PATTERN 3: Vérification rapide ===\n");
    
    Cinema* cinema = cinema_create(10);
    
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    
    // Vérifier tous les 100ms (au lieu de 1000ms par défaut)
    // Idéal pour les tests ou applications temps réel
    capacity_monitor_start(monitor, 100);
    
    // ... Votre code ici ...
    
    capacity_monitor_stop(monitor);
    capacity_monitor_destroy(monitor);
}

// ============================================================
// 5. PATTERN: Intervalle de vérification long
// ============================================================

void pattern_slow_checking() {
    printf("\n=== PATTERN 4: Vérification lente ===\n");
    
    Cinema* cinema = cinema_create(10);
    
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    
    // Vérifier toutes les 5 secondes
    // Moins de ressources CPU consommées
    capacity_monitor_start(monitor, 5000);
    
    // ... Votre code ici ...
    
    capacity_monitor_stop(monitor);
    capacity_monitor_destroy(monitor);
}

// ============================================================
// 6. PATTERN: Gestion d'erreurs robuste
// ============================================================

int pattern_with_error_handling() {
    printf("\n=== PATTERN 5: Avec gestion d'erreurs ===\n");
    
    Cinema* cinema = cinema_create(10);
    if (!cinema) {
        fprintf(stderr, "Erreur: impossible de créer le cinéma\n");
        return -1;
    }
    
    // Initialiser
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    if (!monitor) {
        fprintf(stderr, "Erreur: impossible d'initialiser le moniteur\n");
        return -1;
    }
    
    // Démarrer
    int result = capacity_monitor_start(monitor, 1000);
    if (result != 0) {
        fprintf(stderr, "Erreur: impossible de démarrer le moniteur\n");
        capacity_monitor_destroy(monitor);
        return -1;
    }
    
    printf("✓ Moniteur lancé avec succès\n");
    
    // ... Votre code ici ...
    
    // Arrêter et nettoyer
    capacity_monitor_stop(monitor);
    capacity_monitor_destroy(monitor);
    
    return 0;
}

// ============================================================
// 7. PATTERN: Plusieurs séances monitées
// ============================================================

void pattern_multiple_screenings() {
    printf("\n=== PATTERN 6: Plusieurs séances ===\n");
    
    Cinema* cinema = cinema_create(10);
    
    // Ajouter plusieurs films et séances
    Room* r1 = room_create("Salle 1", 10, 10);
    Movie* m1 = movie_create(1, "Film A", 120, AGE_16, "Action");
    Screening* s1 = screening_create(m1, r1, time(NULL) + 3600, 15.0);
    
    Room* r2 = room_create("Salle 2", 8, 8);
    Movie* m2 = movie_create(2, "Film B", 90, AGE_12, "Comédie");
    Screening* s2 = screening_create(m2, r2, time(NULL) + 5400, 12.0);
    
    cinema->rooms[cinema->num_rooms++] = r1;
    cinema->rooms[cinema->num_rooms++] = r2;
    
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 2));
    cinema->movies[cinema->num_movies++] = m1;
    cinema->movies[cinema->num_movies++] = m2;
    
    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 2));
    cinema->screenings[cinema->num_screenings++] = s1;
    cinema->screenings[cinema->num_screenings++] = s2;
    
    // Un seul moniteur surveille TOUTES les séances
    CapacityMonitor* monitor = capacity_monitor_init(cinema);
    capacity_monitor_start(monitor, 1000);
    
    printf("Moniteur lancé pour 2 séances\n");
    
    // ... Votre code ici ...
    // À chaque fois qu'une séance atteint 90%, une alerte est envoyée
    
    capacity_monitor_stop(monitor);
    capacity_monitor_destroy(monitor);
}

// ============================================================
// 8. PATTERN: Vérification manuelle sans thread
// ============================================================

void pattern_manual_checking() {
    printf("\n=== PATTERN 7: Vérification manuelle ===\n");
    
    Cinema* cinema = cinema_create(10);
    
    // Créer une séance
    Room* r = room_create("Salle Test", 5, 5);
    Movie* m = movie_create(1, "Test", 100, AGE_16, "Test");
    Screening* s = screening_create(m, r, time(NULL) + 3600, 10.0);
    
    cinema->rooms[cinema->num_rooms++] = r;
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * 1);
    cinema->movies[0] = m;
    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * 1);
    cinema->screenings[0] = s;
    
    // Vérification MANUELLE sans thread
    // (Utile si vous voulez contrôler quand vérifier)
    
    // Simule des achats
    for (int i = 0; i < 25; i++) {
        // Acheter un billet
        // ... code d'achat ...
        
        // Vérifier capacité MAINTENANT (pas besoin de thread)
        check_full_capacity_and_alert(cinema, s->id);
        
        // Afficher occupancy
        float occ = cinema->statistics->occupancy_rate_by_screening[s->id];
        printf("Occupancy: %.1f%%\n", occ);
    }
}

// ============================================================
// FONCTION PRINCIPALE: Démo de tous les patterns
// ============================================================

int main() {
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  EXEMPLES: Comment utiliser le CapacityMonitor           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    
    printf("\nCe fichier montre 7 patterns différents d'utilisation.\n");
    printf("Décommentez le pattern que vous voulez tester:\n\n");
    
    // Décommentez le pattern à tester:
    
    // pattern_basic_usage();
    // pattern_with_signal_handler();
    // pattern_fast_checking();
    // pattern_slow_checking();
    // pattern_with_error_handling();
    // pattern_multiple_screenings();
    // pattern_manual_checking();
    
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  Voir les commentaires du code pour chaque pattern        ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}

/*
 * RÉSUMÉ DES 7 PATTERNS:
 * 
 * 1. BASIQUE
 *    - Initialiser → Démarrer → Arrêter → Nettoyer
 *    - Plus simple possible
 * 
 * 2. AVEC HANDLER SIGNAL
 *    - Réagir aux alertes SIGUSR2
 *    - Déclencher des actions spéciales
 * 
 * 3. VÉRIFICATION RAPIDE (100ms)
 *    - Tests ou applications temps réel
 *    - Plus de ressources CPU
 * 
 * 4. VÉRIFICATION LENTE (5000ms)
 *    - Production avec peu de trafic
 *    - Moins de ressources CPU
 * 
 * 5. GESTION D'ERREURS
 *    - Vérifier chaque opération
 *    - Pattern de production robuste
 * 
 * 6. PLUSIEURS SÉANCES
 *    - Un moniteur pour plusieurs séances
 *    - Chaque séance a son propre flag d'alerte
 * 
 * 7. VÉRIFICATION MANUELLE
 *    - Sans thread, vérifier à la demande
 *    - Plus de contrôle mais plus de code
 */
