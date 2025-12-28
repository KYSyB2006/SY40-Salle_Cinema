#ifndef CINEMA_UI_H
#define CINEMA_UI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// --- CONSTANTES ---
#define MAX_NAME 50
#define LOG_FILE "cinema_activity.log"
#define REPORT_FILE "cinema_stats.log"
#define ROWS 5
#define COLS 10
#define PRIX_BILLET 12.50

// Codes Couleurs (ANSI) pour un affichage PRO
#define COL_RESET "\033[0m"
#define COL_RED "\033[0;31m"
#define COL_GREEN "\033[0;32m"
#define COL_YELLOW "\033[0;33m"
#define COL_CYAN "\033[0;36m"

// --- STRUCTURES DE DONNEES ---

// Types de clients pour la priorite
typedef enum {
    TYPE_NORMAL,
    TYPE_VIP // Avec reservation
} ClientType;

// Statistiques
typedef struct {
    int tickets_vendus;
    int reservations_faites;
    int echanges_effectues;
    double recette_totale;
    int clients_vip_servis;
    int clients_normal_servis;
} CinemaStats;

// Noeud de la file d'attente
typedef struct Client {
    int id;
    char nom[MAX_NAME];
    ClientType type; // NORMAL ou VIP
    struct Client* next;
} Client;

// --- VARIABLES GLOBALES PARTAGEES ---
// (Definies dans les .c, declarees ici pour etre visibles partout)
extern char seat_map[ROWS][COLS];
extern CinemaStats current_session;

// --- PROTOTYPES ---

// Dans queue_manager.c
void enqueue_client(Client** head, int id, char* name, ClientType type);
void show_queue_status(Client* head);
void notify_all_clients(Client* head, const char* message);

// Dans reporting.c
void update_stats(double amount, bool is_vip, bool is_exchange);
void ecrire_log(const char* action, const char* details);
void print_daily_report();

// Dans main_ui.c (Utilitaires)
void vider_buffer();


// --- Utilitaires Avancés ---
int lire_entier(const char* message, int min, int max); // Saisie blindée
void simuler_impression_ticket(char rang, int num, double prix); // Joli ticket
void nettoyer_memoire(Client* head); // Anti-fuite mémoire

#endif