#include "cinema_ui.h"

// Matrice globale
char seat_map[ROWS][COLS];

// --- 1. OUTILS AVANCES (Le "Framework" interne) ---

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// FONCTION SENIOR : Gestion centralisée des saisies
// Plus besoin de répéter les scanf et les vérifications partout !
int lire_entier(const char* message, int min, int max) {
    int valeur;
    char buffer[100];
    
    while (1) {
        printf("%s (%d-%d) : ", message, min, max);
        
        // Lecture sécurisée via fgets (évite les bugs de scanf infini)
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Tentative de conversion
            if (sscanf(buffer, "%d", &valeur) == 1) {
                // Vérification des bornes
                if (valeur >= min && valeur <= max) {
                    return valeur; // Succès
                }
                printf(COL_RED ">> Erreur : Valeur hors limites.\n" COL_RESET);
            } else {
                printf(COL_RED ">> Erreur : Ce n'est pas un nombre.\n" COL_RESET);
            }
        }
    }
}

// FONCTION SENIOR : Joli ticket visuel
void simuler_impression_ticket(char rang, int num, double prix) {
    printf("\n");
    printf(COL_CYAN "   /==============================\\\n");
    printf("   |      CINEMA MAXI-PLEX        |\n");
    printf("   |------------------------------|\n");
    printf("   | SEANCE : 20:30               |\n");
    printf("   | SALLE  : 01                  |\n");
    printf("   | PLACE  : %c - %02d             |\n", rang, num);
    printf("   | PRIX   : %5.2f EUR          |\n", prix);
    printf("   |                              |\n");
    printf("   |     Bonne Seance !           |\n");
    printf("   \\==============================/\n" COL_RESET);
    printf("\n");
}

void pause_ecran() {
    printf("\nAppuyez sur Entree pour continuer...");
    getchar(); // Attend juste une frappe (buffer déjà vide grâce à fgets)
}

// --- 2. LOGIQUE SALLE ---

void init_room() {
    for(int i=0; i<ROWS; i++) 
        for(int j=0; j<COLS; j++) seat_map[i][j] = 'O';
    
    seat_map[0][5] = 'X'; // Simulation
    seat_map[2][3] = 'X';
    ecrire_log("SYSTEM", "Salle initialisee");
}

void display_seats() {
    printf("\n      " COL_CYAN "[ ECRAN DE CINEMA ]" COL_RESET "\n");
    printf("      -------------------\n");
    printf("      1 2 3 4 5 6 7 8 9 10\n");
    
    for(int i=0; i<ROWS; i++) {
        printf(" %c |  ", 'A'+i);
        for(int j=0; j<COLS; j++) {
            if(seat_map[i][j] == 'O') printf(COL_GREEN "O " COL_RESET);
            else printf(COL_RED "X " COL_RESET);
        }
        printf("\n");
    }
    printf("\n (" COL_GREEN "O=Libre" COL_RESET ", " COL_RED "X=Occupe" COL_RESET ")\n");
}

int process_booking(char r_char, int c_num, const char* contexte) {
    if(r_char >= 'a' && r_char <= 'z') r_char -= 32;
    int r = r_char - 'A';
    int c = c_num - 1;

    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) {
        printf(COL_RED ">> Erreur: Place inexistante.\n" COL_RESET);
        return 0;
    }
    if (seat_map[r][c] == 'X') {
        printf(COL_RED ">> Erreur: Place deja prise.\n" COL_RESET);
        return 0;
    }

    seat_map[r][c] = 'X';
    
    char msg[100];
    sprintf(msg, "Place %c%d reservee (%s)", r_char, c_num, contexte);
    ecrire_log("RESERVATION", msg);
    return 1;
}

// --- 3. MENUS (Code Clarifié) ---

void kiosk_flow() {
    clear_screen();
    printf(COL_YELLOW "=== GUICHET AUTOMATIQUE ===" COL_RESET "\n");
    display_seats();
    
    // Saisie manuelle de la lettre (cas particulier car c'est un char)
    printf("\nRangee (A-E) : ");
    char r; 
    // Utilisation de fgets pour le char aussi pour être cohérent
    char buf[10]; fgets(buf, sizeof(buf), stdin); r = buf[0];
    
    // Utilisation de notre super fonction
    int n = lire_entier("Numero", 1, 10);

    if (process_booking(r, n, "Guichet")) {
        simuler_impression_ticket(r, n, PRIX_BILLET);
        update_stats(PRIX_BILLET, false, false);
    }
    pause_ecran();
}

void hostess_flow() {
    clear_screen();
    printf(COL_YELLOW "=== INTERFACE HOTESSE ===" COL_RESET "\n");
    printf("1. Vente Assistee\n");
    printf("2. Echange\n");
    printf("0. Retour\n");
    
    int ch = lire_entier("Choix", 0, 2); // <- Regarde comme c'est propre !

    if (ch == 1) {
        printf("\n[SEC] Client +16 ans ? (o/n) : ");
        char buf[10]; fgets(buf, sizeof(buf), stdin); 
        
        if (buf[0] == 'o' || buf[0] == 'O') {
             display_seats();
             printf("Rangee : "); fgets(buf, sizeof(buf), stdin); char r = buf[0];
             int n = lire_entier("Numero", 1, 10);
             
             if(process_booking(r, n, "Hotesse")) {
                 simuler_impression_ticket(r, n, PRIX_BILLET);
                 update_stats(PRIX_BILLET, false, false);
             }
        } else {
             printf(COL_RED ">> REFUS AGE.\n" COL_RESET);
        }
    } 
    else if (ch == 2) {
        char buf[10];
        int id = lire_entier("ID Ancien Billet", 1, 9999);
        printf(">> Billet #%d verifie.\n", id);
        display_seats();
        
        printf("Nouvelle Rangee : "); fgets(buf, sizeof(buf), stdin); char nr = buf[0];
        int nc = lire_entier("Nouveau Numero", 1, 10);
        
        if (process_booking(nr, nc, "Echange")) {
            printf(COL_GREEN ">> ECHANGE VALIDE.\n" COL_RESET);
            simuler_impression_ticket(nr, nc, 0.0);
            update_stats(0.0, false, true);
        }
    }
    pause_ecran();
}

void admin_flow(Client* queue) {
    clear_screen();
    printf(COL_YELLOW "=== ADMINISTRATION ===" COL_RESET "\n");
    printf("1. Rapport Stats\n");
    printf("2. Voir File\n");
    printf("3. Notifier Clients\n");
    printf("0. Retour\n");
    
    int ch = lire_entier("Choix", 0, 3);

    switch(ch) {
        case 1: print_daily_report(); break;
        case 2: show_queue_status(queue); break;
        case 3: notify_all_clients(queue, "ALERTE SALLE 4"); break;
    }
    pause_ecran();
}

// --- MAIN ---

int main() {
    init_room();
    Client* queue = NULL;

    printf("[BOOT] Chargement...\n");
    enqueue_client(&queue, 101, "Jean", TYPE_NORMAL);
    enqueue_client(&queue, 102, "Marie", TYPE_VIP);

    int running = 1;
    while(running) {
        clear_screen();
        printf(COL_CYAN "=== CINEMA MANAGER PRO v2.0 ===" COL_RESET "\n");
        printf("1. Guichet\n");
        printf("2. Hotesse\n");
        printf("3. Admin\n");
        printf("0. Quitter\n");
        
        int role = lire_entier("Votre Role", 0, 3); // Saisie sécurisée ici aussi

        switch(role) {
            case 1: kiosk_flow(); break;
            case 2: hostess_flow(); break;
            case 3: admin_flow(queue); break;
            case 0: running = 0; break;
        }
    }
    
    // NETTOYAGE FINAL (Tres important pour un Senior Dev)
    printf("\n[SHUTDOWN] Nettoyage memoire...\n");
    nettoyer_memoire(queue);
    printf("Au revoir.\n");
    return 0;
}