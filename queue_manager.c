#include "cinema_ui.h"

// Ajoute un client dans la file avec gestion de priorite stricte
void enqueue_client(Client** head, int id, char* name, ClientType type) {
    Client* new_client = (Client*)malloc(sizeof(Client));
    if (!new_client) {
        printf(COL_RED "ERREUR FATALE: Plus de memoire.\n" COL_RESET);
        exit(1);
    }

    new_client->id = id;
    strncpy(new_client->nom, name, MAX_NAME);
    new_client->type = type;
    new_client->next = NULL;

    // Cas 1: Liste vide
    if (*head == NULL) {
        *head = new_client;
        return;
    }

    // Cas 2: Insertion en tete (Si nouveau est VIP et tete est NORMAL)
    if (type == TYPE_VIP && (*head)->type == TYPE_NORMAL) {
        new_client->next = *head;
        *head = new_client;
        return;
    }

    // Cas 3: Insertion intelligente
    Client* current = *head;
    
    if (type == TYPE_VIP) {
        // Avancer tant qu'on est chez les VIPs pour se mettre apres le dernier VIP
        while (current->next != NULL && current->next->type == TYPE_VIP) {
            current = current->next;
        }
    } else {
        // Normal : on va tout au fond
        while (current->next != NULL) {
            current = current->next;
        }
    }

    // Insertion
    new_client->next = current->next;
    current->next = new_client;
    
    // Petit log console pour confirmer
    printf(" > [FILE] Client %s (%s) ajoute.\n", name, type == TYPE_VIP ? "VIP" : "STD");
}

void show_queue_status(Client* head) {
    printf("\n--- ETAT DE LA FILE D'ATTENTE ---\n");
    if (!head) { printf("(File vide)\n"); return; }
    
    Client* c = head;
    int i = 1;
    while(c) {
        printf(" %d. [%s] %-10s (ID: %d)\n", i++, c->type == TYPE_VIP ? "VIP" : "STD", c->nom, c->id);
        c = c->next;
    }
    printf("---------------------------------\n");
}

void notify_all_clients(Client* head, const char* message) {
    printf(COL_YELLOW "\n[NOTIFICATION SYSTEME] Envoi en cours...\n" COL_RESET);
    
    if (!head) {
        printf(" >> Personne a notifier.\n");
        return;
    }

    Client* c = head;
    while(c) {
        // Simulation d'envoi SMS
        printf(" -> Envoi SMS a %s (ID:%d) : '%s'\n", c->nom, c->id, message);
        c = c->next;
    }
    
    // On loggue l'action
    ecrire_log("NOTIF_MASS", message);
    printf(COL_GREEN ">> Tous les clients ont ete notifies.\n" COL_RESET);
}

// Fonction AVANCEE : On rend la mémoire prêtée par le système
void nettoyer_memoire(Client* head) {
    Client* current = head;
    Client* next_node;
    int count = 0;

    while (current != NULL) {
        next_node = current->next; // On sauvegarde l'adresse du suivant
        free(current);             // On supprime l'actuel
        current = next_node;       // On avance
        count++;
    }
    printf(" >> %d clients supprimes de la memoire RAM.\n", count);
}