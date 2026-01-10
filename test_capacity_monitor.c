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

// Compteur pour les alertes SIGUSR2
static int sigusr2_count = 0;

// Handler pour SIGUSR2 (Alerte capacité 90%)
void handle_sigusr2(int sig) {
    (void)sig;  // Éviter le warning unused parameter
    sigusr2_count++;
    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║ [ALERTE CAPACITE] Signal SIGUSR2 recu   ║\n");
    printf("║ Nombre d'alertes: %d                     ║\n", sigusr2_count);
    printf("╚═══════════════════════════════════════════╝\n\n");
}

// Structure pour passer les paramètres aux threads d'achat
typedef struct {
    Cinema* cinema;
    int screening_id;
    int num_tickets;
    int seat_offset;
} BuyerThreadArgs;

// Thread d'achat de billets
void* buyer_thread(void* arg) {
    BuyerThreadArgs* args = (BuyerThreadArgs*)arg;
    Cinema* cinema = args->cinema;
    int screening_id = args->screening_id;
    int num_tickets = args->num_tickets;
    int seat_offset = args->seat_offset;
    
    AlternativeList* alt = NULL;
    
    for (int i = 0; i < num_tickets; i++) {
        int seat_id = seat_offset + i;
        
        TicketResult result = purchase_ticket(cinema, screening_id, 
                                             "Client", "client@cinema.fr", 
                                             20, seat_id, &alt);
        
        if (result == OK) {
            printf("✓ Billet %d acheté pour la séance %d (Siège %d)\n", 
                   i + 1, screening_id, seat_id);
        } else if (result == SEAT_UNAVAILABLE) {
            printf("✗ Billet %d refusé: siège indisponible ou capacité pleine (Seance %d)\n", 
                   i + 1, screening_id);
            break;
        } else {
            printf("✗ Billet %d refusé (code: %d) (Seance %d)\n", 
                   i + 1, result, screening_id);
        }
        
        sleep(1);  // Pause entre chaque achat pour voir les alertes
    }
    
    free(args);
    return NULL;
}

int main() {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   TEST DU CAPACITY MONITOR - ALERTE 90 POUR CENT         ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    // Enregistrer le handler pour SIGUSR2
    signal(SIGUSR2, handle_sigusr2);
    
    srand(time(NULL));
    
    // ========== INITIALISATION DU CINÉMA ==========
    printf("[SETUP] Initialisation du cinéma...\n");
    Cinema* cinema = cinema_create(5);
    
    // Créer une salle 2x5 (10 places)
    Room* r1 = room_create("Salle Premium", 2, 5);
    Movie* m1 = movie_create(1, "Matrix Reloaded", 120, AGE_16, "Action");
    Screening* s1 = screening_create(m1, r1, time(NULL) + 3600, 15.0);
    
    cinema->rooms[cinema->num_rooms++] = r1;
    cinema->movies = realloc(cinema->movies, sizeof(Movie*) * (cinema->num_movies + 1));
    cinema->movies[cinema->num_movies++] = m1;
    cinema->screenings = realloc(cinema->screenings, sizeof(Screening*) * (cinema->num_screenings + 1));
    cinema->screenings[cinema->num_screenings++] = s1;
    
    printf("✓ Salle créée: '%s' (10 places)\n", r1->name);
    printf("✓ Film créé: '%s' (Catégorie: %s)\n", m1->title, age_rating_to_string(m1->age_rating));
    printf("✓ Seance creee (ID: %d)\n\n", s1->id);
    
    // ========== INITIALISATION DU MONITEUR ==========
    printf("[SETUP] Initialisation du moniteur de capacité...\n");
    CapacityMonitor* capacity_monitor = capacity_monitor_init(cinema);
    if (!capacity_monitor) {
        fprintf(stderr, "Erreur: impossible d'initialiser le moniteur\n");
        return -1;
    }
    
    printf("✓ Moniteur de capacité initialisé\n");
    
    // Démarrer le monitoring (vérification chaque 500ms)
    int result = capacity_monitor_start(capacity_monitor, 500);
    if (result != 0) {
        fprintf(stderr, "Erreur: impossible de démarrer le moniteur\n");
        capacity_monitor_destroy(capacity_monitor);
        return -1;
    }
    
    printf("✓ Moniteur demarré (intervalle: 500ms)\n");
    printf("✓ Seuil d'alerte: %.1f POUR CENT de capacité\n\n", FULL_CAPACITY_THRESHOLD);
    
    // ========== TEST: ACHATS DE BILLETS ==========
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE DE TEST: ACHATS PROGRESSIFS DE BILLETS            ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Nous allons acheter 9 billets sur 10 places disponibles.\n");
    printf("A 90 POUR CENT de capacité (9 places), une alerte SIGUSR2 doit etre envoyee.\n\n");
    
    // Créer un thread d'achat qui va remplir la salle
    pthread_t buyer;
    BuyerThreadArgs* args = malloc(sizeof(BuyerThreadArgs));
    args->cinema = cinema;
    args->screening_id = s1->id;
    args->num_tickets = 9;
    args->seat_offset = 1;
    
    pthread_create(&buyer, NULL, buyer_thread, args);
    
    // Attendre que le thread se termine
    pthread_join(buyer, NULL);
    
    // ========== AFFICHAGE DES STATISTIQUES ==========
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    RÉSULTATS DU TEST                      ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    // Attendre un peu pour que le moniteur envoie les dernières alertes
    sleep(2);
    
    printf("📊 Statistiques de la séance:\n");
    printf("  • Billets vendus: %d / 10\n", s1->seats_sold);
    printf("  • Places disponibles: %d\n", r1->available_seats);
    printf("  • Taux d'occupation: %.1f%%\n", 
           cinema->statistics->occupancy_rate_by_screening[s1->id]);
    printf("  • Total billets vendus (cinéma): %d\n", cinema->statistics->total_tickets_sold);
    printf("  • Revenu total: %.2f€\n\n", cinema->statistics->total_revenue);
    
    printf("🚨 Alertes SIGUSR2 reçues: %d\n", sigusr2_count);
    
    if (sigusr2_count > 0) {
        printf("✓ TEST RÉUSSI: L'alerte capacité a été correctement déclenchée!\n");
    } else {
        printf("⚠ ATTENTION: Aucune alerte n'a été reçue\n");
    }
    
    // ========== TEST: TENTATIVE D'ACHAT À CAPACITÉ PLEINE ==========
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PHASE DE TEST: TENTATIVE D'ACHAT A 90+ POUR CENT        ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Tentative d'achat d'un 10e billet (la salle est a 90+)...\n");
    
    AlternativeList* alt2 = NULL;
    // Utiliser le siège 0 qui devrait être disponible puisque nous avons acheté les sièges 1-9
    TicketResult final_result = purchase_ticket(cinema, s1->id, 
                                                "Client", "client@cinema.fr", 
                                                20, 0, &alt2);
    
    printf("\nRésultat: ");
    switch(final_result) {
        case OK:
            printf("✗ Billet VENDU (ne devrait pas l'être)\n");
            break;
        case SEAT_UNAVAILABLE:
            printf("✓ Achat REFUSÉ (comportement attendu)\n");
            break;
        case AGE_DENIED:
            printf("✗ Refusé pour restriction d'âge\n");
            break;
        default:
            printf("Code d'erreur: %d\n", final_result);
    }
    
    // ========== NETTOYAGE ==========
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                    NETTOYAGE                              ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    printf("Arrêt du moniteur de capacité...\n");
    capacity_monitor_stop(capacity_monitor);
    printf("✓ Moniteur arrêté\n");
    
    printf("Libération des ressources...\n");
    capacity_monitor_destroy(capacity_monitor);
    printf("✓ Ressources du moniteur libérées\n");
    
    printf("\n✓ Test terminé avec succès!\n\n");
    
    return 0;
}
