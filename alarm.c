#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "alarm.h"

static int compare_desc(const void* a, const void* b) {
    const ScreeningRanking* ra = (const ScreeningRanking*)a;
    const ScreeningRanking* rb = (const ScreeningRanking*)b;
    if (rb->occupancy_rate > ra->occupancy_rate) return 1;
    if (rb->occupancy_rate < ra->occupancy_rate) return -1;
    return 0;
}

const char* age_rating_to_string(AgeRating age_rating) {
    switch (age_rating) {
        case AGE_ALL: return "Tous publics";
        case AGE_12:  return "-12 ans";
        case AGE_16:  return "-16 ans";
        case AGE_18:  return "-18 ans";
        default:      return "Inconnu";
    }
}

static int rank_by_age(Cinema* cinema, AgeRating age, ScreeningRanking* rankings, int max) {
    if (!cinema || !cinema->statistics || !rankings || max <= 0) return -1;
    
    int count = 0;
    for (int i = 0; i < MAX_SCREENINGS && count < max; i++) {
        Screening* s = NULL;
        for (int j = 0; j < cinema->num_screenings; j++) {
            if (cinema->screenings[j] && cinema->screenings[j]->id == i) {
                s = cinema->screenings[j];
                break;
            }
        }
        if (s && s->movie && s->movie->age_rating == age) {
            rankings[count].screening_id = i;
            rankings[count].occupancy_rate = cinema->statistics->tickets_by_movie[s->movie->id];
            rankings[count].age_rating = age;
            rankings[count].screening = s;
            count++;
        }
    }
    if (count > 1) qsort(rankings, count, sizeof(ScreeningRanking), compare_desc);
    return count;
}

static void print_ranking(Cinema* cinema, AgeRating age, int top_n) {
    if (!cinema || !cinema->statistics) return;
    
    ScreeningRanking* r = malloc(MAX_SCREENINGS * sizeof(ScreeningRanking));
    if (!r) return;
    
    int count = rank_by_age(cinema, age, r, MAX_SCREENINGS);
    if (count <= 0) { free(r); return; }
    
    int n = (top_n > 0 && top_n < count) ? top_n : count;
    printf("\n=== %s ===\n", age_rating_to_string(age));
    for (int i = 0; i < n; i++) {
        printf("%d. ID:%d %s %.2f%%\n", i+1, r[i].screening_id,
               r[i].screening && r[i].screening->movie ? r[i].screening->movie->title : "N/A",
               r[i].occupancy_rate);
    }
    free(r);
}

int rank_screenings_age_all(Cinema* c, ScreeningRanking* r, int m) { return rank_by_age(c, AGE_ALL, r, m); }
int rank_screenings_age_12(Cinema* c, ScreeningRanking* r, int m) { return rank_by_age(c, AGE_12, r, m); }
int rank_screenings_age_16(Cinema* c, ScreeningRanking* r, int m) { return rank_by_age(c, AGE_16, r, m); }
int rank_screenings_age_18(Cinema* c, ScreeningRanking* r, int m) { return rank_by_age(c, AGE_18, r, m); }

void print_ranking_age_all(Cinema* c, int n) { print_ranking(c, AGE_ALL, n); }
void print_ranking_age_12(Cinema* c, int n) { print_ranking(c, AGE_12, n); }
void print_ranking_age_16(Cinema* c, int n) { print_ranking(c, AGE_16, n); }
void print_ranking_age_18(Cinema* c, int n) { print_ranking(c, AGE_18, n); }

Movie* find_most_popular_movie_by_age(Cinema* cinema, AgeRating age) {
    if (!cinema || !cinema->statistics) return NULL;
    
    ScreeningRanking* r = malloc(MAX_SCREENINGS * sizeof(ScreeningRanking));
    if (!r) return NULL;
    
    int count = rank_by_age(cinema, age, r, MAX_SCREENINGS);
    Movie* m = (count > 0 && r[0].screening && r[0].screening->movie) ? r[0].screening->movie : NULL;
    free(r);
    return m;
}

// Fonction interne pour notifier les utilisateurs concernés
static int notify_users_for_screening(Cinema* cinema, int screening_id,
                                       const char* old_title, const char* new_title) {
    int notified = 0;
    
    // Notifier les utilisateurs avec des tickets vendus
    for (int i = 0; i < cinema->num_tickets; i++) {
        Ticket* t = cinema->tickets[i];
        if (t && t->screening && t->screening->id == screening_id) {
            if (t->status == TICKET_SOLD || t->status == TICKET_VALID) {
                // Simulation de notification (email)
                printf("[NOTIFICATION] %s (%s): Séance %d - \"%s\" remplacé par \"%s\"\n",
                       t->customer_name, t->email, screening_id, old_title, new_title);
                notified++;
            }
        }
    }
    
    // Notifier les utilisateurs avec des réservations
    if (cinema->reservation_list) {
        TicketNode* node = cinema->reservation_list->head;
        while (node) {
            Ticket* t = node->ticket;
            if (t && t->screening && t->screening->id == screening_id && t->status == TICKET_VALID) {
                printf("[NOTIFICATION] %s (%s): Séance %d - \"%s\" remplacé par \"%s\"\n",
                       t->customer_name, t->email, screening_id, old_title, new_title);
                notified++;
            }
            node = node->next;
        }
    }
    
    return notified;
}

int replace_movie_with_more_popular(Cinema* cinema, int screening_id) {
    if (!cinema || !cinema->statistics || !cinema->screenings) return -1;
    
    Screening* target = NULL;
    for (int i = 0; i < cinema->num_screenings; i++) {
        if (cinema->screenings[i] && cinema->screenings[i]->id == screening_id) {
            target = cinema->screenings[i];
            break;
        }
    }
    if (!target || !target->movie) return -1;
    
    // Sauvegarder l'ancien titre
    char old_title[100];
    strncpy(old_title, target->movie->title, sizeof(old_title) - 1);
    old_title[sizeof(old_title) - 1] = '\0';
    
    AgeRating age = target->movie->age_rating;
    float current_occ = cinema->statistics->occupancy_rate_by_screening[screening_id];
    
    ScreeningRanking* r = malloc(MAX_SCREENINGS * sizeof(ScreeningRanking));
    if (!r) return -1;
    
    int count = rank_by_age(cinema, age, r, MAX_SCREENINGS);
    Movie* better = NULL;
    
    for (int i = 0; i < count; i++) {
        if (r[i].screening && r[i].screening->movie &&
            r[i].screening->movie->id != target->movie->id &&
            r[i].occupancy_rate > current_occ) {
            better = r[i].screening->movie;
            break;
        }
    }
    free(r);
    
    if (!better) return -2;
    
    // Remplacer le film
    target->movie = better;
    
    // Générer l'alarme SIGUSR1
    kill(getpid(), SIGUSR1);
    
    // Notifier les utilisateurs concernés
    notify_users_for_screening(cinema, screening_id, old_title, better->title);
    
    return 0;
}

// Vérifie si une séance a atteint 90% de capacité et envoie une alerte
int check_full_capacity_and_alert(Cinema* cinema, int screening_id) {
    if (!cinema || !cinema->statistics || !cinema->screenings) return -1;
    
    // Vérifier que le screening_id est valide
    if (screening_id < 0 || screening_id >= MAX_SCREENINGS) return -1;
    
    // Récupérer la occupancy rate pour cette séance
    float occupancy = cinema->statistics->occupancy_rate_by_screening[screening_id];
    
    // Vérifier si le seuil de 90% est atteint
    if (occupancy >= FULL_CAPACITY_THRESHOLD) {
        // Récupérer la séance correspondante
        Screening* screening = NULL;
        for (int i = 0; i < cinema->num_screenings; i++) {
            if (cinema->screenings[i] && cinema->screenings[i]->id == screening_id) {
                screening = cinema->screenings[i];
                break;
            }
        }
        
        if (!screening || !screening->movie || !screening->room) return -1;
        
        // Envoyer le signal SIGUSR2
        printf("[ALERTE CAPACITÉ] Séance %d - Salle %s (Film: \"%s\") a atteint %.1f%% de capacité. Vente des billets BLOQUEÉE.\n",
               screening_id, screening->room->name, screening->movie->title, occupancy);
        kill(getpid(), SIGUSR2);
        
        return 1;  // Alerte envoyée avec succès
    }
    
    return 0;  // Seuil non atteint
}

int update_can_change_all(Cinema* cinema) {
    if (!cinema || !cinema->statistics || !cinema->screenings) return -1;
    
    int modified = 0;
    for (int i = 0; i < cinema->num_screenings; i++) {
        Screening* s = cinema->screenings[i];
        if (s) {
            float occ = cinema->statistics->occupancy_rate_by_screening[s->id];
            int new_val = (occ > OCCUPANCY_THRESHOLD) ? 0 : 1;
            if (s->can_change != new_val) {
                s->can_change = new_val;
                modified++;
            }
        }
    }
    return modified;
}

static void* monitor_thread(void* arg) {
    OccupancyMonitor* m = (OccupancyMonitor*)arg;
    while (m->active) {
        pthread_mutex_lock(&m->mutex);
        update_can_change_all(m->cinema);
        pthread_mutex_unlock(&m->mutex);
        
        struct timespec ts = {m->interval_ms / 1000, (m->interval_ms % 1000) * 1000000L};
        nanosleep(&ts, NULL);
    }
    return NULL;
}

OccupancyMonitor* occupancy_monitor_init(Cinema* cinema) {
    if (!cinema) return NULL;
    
    OccupancyMonitor* m = malloc(sizeof(OccupancyMonitor));
    if (!m) return NULL;
    
    m->cinema = cinema;
    m->active = 0;
    m->interval_ms = 1000;
    pthread_mutex_init(&m->mutex, NULL);
    return m;
}

int occupancy_monitor_start(OccupancyMonitor* m, int interval_ms) {
    if (!m || m->active) return -1;
    m->interval_ms = interval_ms > 0 ? interval_ms : 1000;
    m->active = 1;
    return pthread_create(&m->thread, NULL, monitor_thread, m) == 0 ? 0 : -1;
}

void occupancy_monitor_stop(OccupancyMonitor* m) {
    if (!m || !m->active) return;
    m->active = 0;
    pthread_join(m->thread, NULL);
}

void occupancy_monitor_destroy(OccupancyMonitor* m) {
    if (!m) return;
    if (m->active) occupancy_monitor_stop(m);
    pthread_mutex_destroy(&m->mutex);
    free(m);
}
// ============================================================
// Surveillance de la capacité à 90%
// ============================================================

static int check_and_alert_all_screenings(Cinema* cinema, CapacityMonitor* monitor) {
    if (!cinema || !cinema->statistics || !cinema->screenings || !monitor) return -1;
    
    int alerts = 0;
    
    for (int i = 0; i < cinema->num_screenings; i++) {
        Screening* s = cinema->screenings[i];
        if (!s) continue;
        
        float occupancy = cinema->statistics->occupancy_rate_by_screening[s->id];
        
        // Si la capacité atteint 90% et qu'on n'a pas encore alerté cette séance
        if (occupancy >= FULL_CAPACITY_THRESHOLD && !monitor->alerted_screenings[s->id]) {
            // Envoyer l'alerte
            check_full_capacity_and_alert(cinema, s->id);
            monitor->alerted_screenings[s->id] = 1;  // Marquer comme alertée
            alerts++;
        }
        // Si la capacité redescend sous 90%, réinitialiser le flag
        else if (occupancy < FULL_CAPACITY_THRESHOLD) {
            monitor->alerted_screenings[s->id] = 0;
        }
    }
    
    return alerts;
}

static void* capacity_monitor_thread(void* arg) {
    CapacityMonitor* m = (CapacityMonitor*)arg;
    
    while (m->active) {
        pthread_mutex_lock(&m->mutex);
        check_and_alert_all_screenings(m->cinema, m);
        pthread_mutex_unlock(&m->mutex);
        
        struct timespec ts = {m->interval_ms / 1000, (m->interval_ms % 1000) * 1000000L};
        nanosleep(&ts, NULL);
    }
    return NULL;
}

CapacityMonitor* capacity_monitor_init(Cinema* cinema) {
    if (!cinema) return NULL;
    
    CapacityMonitor* m = malloc(sizeof(CapacityMonitor));
    if (!m) return NULL;
    
    m->cinema = cinema;
    m->active = 0;
    m->interval_ms = 1000;  // Vérification toutes les 1 seconde par défaut
    pthread_mutex_init(&m->mutex, NULL);
    
    // Initialiser le tableau des alertes
    memset(m->alerted_screenings, 0, sizeof(m->alerted_screenings));
    
    return m;
}

int capacity_monitor_start(CapacityMonitor* m, int interval_ms) {
    if (!m || m->active) return -1;
    
    m->interval_ms = interval_ms > 0 ? interval_ms : 1000;
    m->active = 1;
    
    return pthread_create(&m->thread, NULL, capacity_monitor_thread, m) == 0 ? 0 : -1;
}

void capacity_monitor_stop(CapacityMonitor* m) {
    if (!m || !m->active) return;
    
    m->active = 0;
    pthread_join(m->thread, NULL);
}

void capacity_monitor_destroy(CapacityMonitor* m) {
    if (!m) return;
    
    if (m->active) capacity_monitor_stop(m);
    pthread_mutex_destroy(&m->mutex);
    free(m);
}
