#ifndef ALARM_H
#define ALARM_H

#include <pthread.h>
#include "struct.h"

#define MAX_SCREENINGS 200
#define OCCUPANCY_THRESHOLD 20.0f
#define FULL_CAPACITY_THRESHOLD 90.0f

typedef struct {
    int screening_id;
    float occupancy_rate;
    AgeRating age_rating;
    Screening* screening;
} ScreeningRanking;

typedef struct {
    Cinema* cinema;
    pthread_t thread;
    pthread_mutex_t mutex;
    int active;
    int interval_ms;
} OccupancyMonitor;

typedef struct {
    Cinema* cinema;
    pthread_t thread;
    pthread_mutex_t mutex;
    int active;
    int interval_ms;
    int alerted_screenings[MAX_SCREENINGS];  // Tracker des séances déjà alertées
} CapacityMonitor;

// Tri par catégorie d'âge
int rank_screenings_age_all(Cinema* cinema, ScreeningRanking* rankings, int max);
int rank_screenings_age_12(Cinema* cinema, ScreeningRanking* rankings, int max);
int rank_screenings_age_16(Cinema* cinema, ScreeningRanking* rankings, int max);
int rank_screenings_age_18(Cinema* cinema, ScreeningRanking* rankings, int max);

// Affichage
void print_ranking_age_all(Cinema* cinema, int top_n);
void print_ranking_age_12(Cinema* cinema, int top_n);
void print_ranking_age_16(Cinema* cinema, int top_n);
void print_ranking_age_18(Cinema* cinema, int top_n);

// Modification de screening
int replace_movie_with_more_popular(Cinema* cinema, int screening_id);
Movie* find_most_popular_movie_by_age(Cinema* cinema, AgeRating age_rating);

// Alarme et notification lors de changement de film
// Remplace le film et notifie tous les utilisateurs ayant un ticket pour ce screening
// Retourne le nombre d'utilisateurs notifiés, -1 en cas d'erreur
int replace_movie_and_notify(Cinema* cinema, int screening_id);

// Surveillance can_change
OccupancyMonitor* occupancy_monitor_init(Cinema* cinema);
int occupancy_monitor_start(OccupancyMonitor* monitor, int interval_ms);
void occupancy_monitor_stop(OccupancyMonitor* monitor);
void occupancy_monitor_destroy(OccupancyMonitor* monitor);
int update_can_change_all(Cinema* cinema);

// Surveillance capacité à 90%
// Thread en continu qui vérifie et alerte quand une séance atteint 90% de capacité
CapacityMonitor* capacity_monitor_init(Cinema* cinema);
int capacity_monitor_start(CapacityMonitor* monitor, int interval_ms);
void capacity_monitor_stop(CapacityMonitor* monitor);
void capacity_monitor_destroy(CapacityMonitor* monitor);

// Alerte capacité complète (90%)
// Envoie un signal SIGUSR2 lorsqu'une séance atteint 90% de remplissage
// Empêche la vente de nouveaux billets pour cette séance
int check_full_capacity_and_alert(Cinema* cinema, int screening_id);

// Utilitaire
const char* age_rating_to_string(AgeRating age_rating);

#endif
