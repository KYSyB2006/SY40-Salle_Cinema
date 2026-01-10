#ifndef THREADS_H
#define THREADS_H

#include "client.h"

typedef struct {
    int client_id;
    char name[50];
    char email[50];
    int age;
    ClientAction action;
    int screening_id;
    int seat_id;
    int ticket_id;        // pour exchange, cancel, refund
    int new_seat_id;
    int new_screening_id;
    int has_reservation;  // priorité
} TicketIntention;

typedef struct TicketIntentionNode {
    TicketIntention* intention;
    struct TicketIntentionNode* next;
} TicketIntentionNode;

typedef struct {
    TicketIntentionNode* head;
    TicketIntentionNode* tail;
    int size;
    pthread_mutex_t mutex;
} TicketIntentionList;

TicketIntentionList* ticketlistint_create(void);
void push_intention(TicketIntentionList* list, TicketIntention* ticket);
TicketIntention* pop_intention(TicketIntentionList* list);

void* hostess_thread(void* arg);
void* kiosk_thread(void* arg);
void* processor_thread(void* arg);
void* client_thread(void* arg);
void* client_thread2(void* arg);
void* supervisor_thread(void* arg);
void* scheduler_thread(void* arg);

#endif