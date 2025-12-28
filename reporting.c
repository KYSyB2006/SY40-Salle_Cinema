#include "cinema_ui.h"

// Definition de la variable globale stats
CinemaStats current_session = {0, 0, 0, 0.0, 0, 0};

// Ecrit une ligne horodatee dans le fichier journal
void ecrire_log(const char* action, const char* details) {
    FILE* f = fopen(LOG_FILE, "a");
    if (f) {
        time_t now = time(NULL);
        char* t_str = ctime(&now);
        t_str[strlen(t_str)-1] = '\0'; // Enlever le saut de ligne auto
        
        fprintf(f, "[%s] ACTION: %-10s | DETAILS: %s\n", t_str, action, details);
        fclose(f);
    }
}

void update_stats(double amount, bool is_vip, bool is_exchange) {
    current_session.recette_totale += amount;
    
    if (is_exchange) current_session.echanges_effectues++;
    else current_session.tickets_vendus++;

    if (is_vip) current_session.reservations_faites++; // Consideré comme VIP/Resa
    
    // Mise a jour compteurs clients
    if (is_vip) current_session.clients_vip_servis++;
    else current_session.clients_normal_servis++;

    // Sauvegarde immediate dans le fichier de stats (persistance)
    FILE *f = fopen(REPORT_FILE, "w");
    if (f) {
        // Cast en (long long) pour eviter warning Windows
        fprintf(f, "LAST_UPDATE: %lld\n", (long long)time(NULL));
        fprintf(f, "TICKETS: %d\n", current_session.tickets_vendus);
        fprintf(f, "RECETTE: %.2f\n", current_session.recette_totale);
        fclose(f);
    }
}

void print_daily_report() {
    float taux = (float)current_session.tickets_vendus / (ROWS * COLS) * 100.0;
    
    printf("\n" COL_CYAN "=== RAPPORT DE PERFORMANCE ===" COL_RESET "\n");
    printf("Billets Vendus     : %d\n", current_session.tickets_vendus);
    printf("Echanges Geres     : %d\n", current_session.echanges_effectues);
    printf("Clients VIP        : %d\n", current_session.clients_vip_servis);
    printf("Clients Standard   : %d\n", current_session.clients_normal_servis);
    printf("Taux Occupation    : %.2f %%\n", taux);
    printf("Recette Totale     : %.2f EUR\n", current_session.recette_totale);
    printf("==============================\n");
}