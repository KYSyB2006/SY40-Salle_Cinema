#ifndef GESTION_H
#define GESTION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include "struct.h"

Cinema* cinema_create(int num_rooms_total);


Room* room_create(const char* name, int rows, int cols);
Movie* movie_create(int id, const char* title, int duration, AgeRating rating, char* genre);
Screening* screening_create(Movie* movie, Room* room, time_t start_time, float price);

int cinema_add_room(Cinema* cinema, const char* name, int rows, int cols);
int cinema_add_movie(Cinema* cinema, const char* title, int duration, AgeRating rating, char* genre);
int cinema_add_screening(Cinema* cinema, int movie_id, int room_id, time_t start_time, float price);

int remove_room(Cinema* cinema, int room_id);
int remove_movie(Cinema* cinema, int movie_id);
int remove_screening(Cinema* cinema, int screening_id);

Room** list_rooms(Cinema* cinema, int* count);
Movie** list_movies(Cinema* cinema, int* count);
Screening** list_screenings(Cinema* cinema, int* count);

float calculate_occupancy(Screening* screening);

TicketList* ticketlist_create(void);
void ticketlist_destroy(TicketList* list);

int check_max_capacity(Screening* screening);
int lock_screening(Screening* screening);
int unlock_screening(Screening* screening);
void notify_admin_threshold(Screening* screening);
int can_switch_film(Screening* screening);
int switch_film(Screening* screening, Movie* new_movie);
void update_dynamic_schedule(Cinema* cinema);

#endif