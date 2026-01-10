#include <stdlib.h>
#include <stdio.h>
#include "threads.h"
#include <unistd.h>
#include "client.h"
#include "ticket_service.h"
#include "reservation_service.h"
#include "alternatives.h"
#include "struct.h"

//création de la liste des intentions
TicketIntentionList* ticketlistint_create() {
    TicketIntentionList* list = malloc(sizeof(TicketIntentionList));
    list->head = list->tail = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL);
    return list;
}

//ajouter une intention
void push_intention(TicketIntentionList* list, TicketIntention* t) {
    TicketIntentionNode* node = malloc(sizeof(TicketIntentionNode));
    node->intention = t;
    node->next = NULL;

    pthread_mutex_lock(&list->mutex);

    // priorité aux réservations
    if (t->has_reservation && list->head) {
        node->next = list->head;
        list->head = node;
    } else {
        if (!list->tail) list->head = list->tail = node;
        else {
            list->tail->next = node;
            list->tail = node;
        }
    }
    list->size++;

    pthread_mutex_unlock(&list->mutex);
}

//retirer une intention
TicketIntention* pop_intention(TicketIntentionList* list) {
    pthread_mutex_lock(&list->mutex);

    if (!list->head) {
        pthread_mutex_unlock(&list->mutex);
        return NULL;
    }

    TicketIntentionNode* node = list->head;
    list->head = node->next;
    if (!list->head) list->tail = NULL;

    list->size--;
    pthread_mutex_unlock(&list->mutex);

    TicketIntention* t = node->intention;
    free(node);
    return t;
}

//thread hôtesse
void* hostess_thread(void* arg) {
    Cinema* cinema = (Cinema*)arg;
    while (1) {
        Client* c = dequeue_client(cinema->client_queue);
        if (!c) continue;

        TicketIntention* t = malloc(sizeof(TicketIntention));
        if (!t) continue;
        t->client_id = c->id;
        snprintf(t->name, sizeof(t->name), "%s", c->name);
        snprintf(t->email, sizeof(t->email), "%s", c->email);
        t->age = c->age;
        t->new_screening_id = c->new_screening_id;
        t->new_seat_id = c->new_seat_id;
        t->action = c->action;
        t->screening_id = c->screening_id;
        t->seat_id = c->seat_id;
        t->ticket_id = c->ticket_id;
        t->has_reservation = c->has_reservation;

        push_intention(cinema->counter_list, t);

        printf("[HOSTESS] Client %d -> intention ajoutée\n", c->id);
        free(c);
        usleep(100000);
    }
}

//thread kiosk
void* kiosk_thread(void* arg) {
    Cinema* cinema = (Cinema*)arg;
    while (1) {
        Client* c = dequeue_client(cinema->client_queue);
        if (!c) continue;

        TicketIntention* t = malloc(sizeof(TicketIntention));
        if (!t) continue;
        t->client_id = c->id;
        snprintf(t->name, sizeof(t->name), "%s", c->name);
        snprintf(t->email, sizeof(t->email), "%s", c->email);
        t->age = c->age;
        t->new_screening_id = c->new_screening_id;
        t->new_seat_id = c->new_seat_id;
        t->action = c->action;
        t->screening_id = c->screening_id;
        t->seat_id = c->seat_id;
        t->ticket_id = c->ticket_id;
        t->has_reservation = c->has_reservation;

        push_intention(cinema->kiosk_list, t);

        printf("[KIOSK] Client %d -> intention ajoutée\n", c->id);
        free(c);
        usleep(100000);
    }
}

//thread processeur
void* processor_thread(void* arg) {
    Cinema* cinema = (Cinema*)arg;
    static int lastqueue = 0; // 0 = counter, 1 = kiosk
    while (1) {
        TicketIntention* t = NULL;
        // alterner entre les deux files d'attente
        if (lastqueue == 0) {
            t = pop_intention(cinema->counter_list);
            if (!t)
                t = pop_intention(cinema->kiosk_list);
        } else {
            t = pop_intention(cinema->kiosk_list);
            if (!t)
                t = pop_intention(cinema->counter_list);
        }
        lastqueue = !lastqueue;

        if (!t) { usleep(50000); continue; }

        int result=1;
        AlternativeList* alt=NULL;
        TicketResult r;

        //achat et echange
        if(t->action == BUY || t->action == EXCHANGE || t->action == RESERVE || t->action == MODIFY_RESERVATION) {
            int retry = 0;
            do {
                retry = 0;

                switch (t->action) {
                case BUY:
                    r = purchase_ticket(cinema, t->screening_id, t->name, t->email, t->age, t->seat_id, &alt);
                
                    if ((r == SEAT_UNAVAILABLE || r == AGE_DENIED) && alt && alt->count > 0 ) {
                        // stratégie simulation : prendre la 1ère alternative
                        t->screening_id = alt->options[0].screening_id;
                        t->seat_id = alt->options[0].seat_id;
                        retry = 1;
                    }
                    result=(r==OK);
                    break;
                case EXCHANGE:
                    r = exchange_ticket(cinema, t->ticket_id, t->new_screening_id, t->new_seat_id, &alt);

                    if ((r == SEAT_UNAVAILABLE || r == AGE_DENIED) && alt && alt->count > 0 ) {
                        // stratégie simulation : prendre la 1ère alternative
                        t->screening_id = alt->options[0].screening_id;
                        t->seat_id = alt->options[0].seat_id;
                        retry = 1;
                    }
                    result=(r==OK);
                    break;
                case RESERVE:
                    r = make_reservation(cinema, t->screening_id, t->name, t->email, t->age, t->seat_id, &alt);
                    if ((r == SEAT_UNAVAILABLE || r == AGE_DENIED) && alt && alt->count > 0 ) {
                        // stratégie simulation : prendre la 1ère alternative
                        t->screening_id = alt->options[0].screening_id;
                        t->seat_id = alt->options[0].seat_id;
                        retry = 1;
                    }
                    result=(r==OK);
                    break;
                case MODIFY_RESERVATION:
                    r = modify_reservation(cinema, t->ticket_id, t->new_screening_id, t->new_seat_id, &alt);
                    if ((r == SEAT_UNAVAILABLE || r == AGE_DENIED) && alt && alt->count > 0 ) {
                        // stratégie simulation : prendre la 1ère alternative
                        t->screening_id = alt->options[0].screening_id;
                        t->seat_id = alt->options[0].seat_id;
                        retry = 1;
                    }
                    result=(r==OK);
                    break;
                default:
                    r = 0;
                    break;
                }
                if (alt) {
                    cinema->statistics->alternatives_generated += alt->count;
                    cinema->statistics->alternatives_used++;
                    free(alt->options);
                    free(alt);
                    alt = NULL;
                }
            } while (retry);
        }
        else{
            switch (t->action) {
                case VALIDATE_RESERVATION:
                    result = validate_reservation(cinema, t->ticket_id);
                    break;
                case CANCEL:
                    result = cancel_ticket(cinema, t->ticket_id);
                    break;
                case CANCEL_RESERVATION:
                    result = cancel_reservation(cinema, t->ticket_id);
                    break;
                case REFUND:
                    result = refund_ticket(cinema, t->ticket_id);
                    break;
                default:
                    break;
            }
        }

        printf("[PROCESSOR] Client %d action %s => %s \n",t->client_id, action_to_string(t->action), result ? "OK" : "FAILED");

        free(t);
        usleep(50000);

    }
}

//client thread pour les tests (client sans réservation)
void* client_thread(void* arg) {
    Cinema* cinema = (Cinema*)arg;

    Client* c = malloc(sizeof(Client));
    if (!c) return NULL;

    c->id = rand() % 100;
    c->age = 10+ rand() % 20;
    c->has_reservation = 0;

    snprintf(c->name, sizeof(c->name), "Client_%d", c->id);
    snprintf(c->email, sizeof(c->email), "client%d@mail.com", c->id);

    int actions = 1 + rand() % 2; // 1 à 3 actions

    for (int i = 0; i < actions; i++) {

        int choice = rand() % 5;

        switch (choice) {
            case 0:
                c->action = BUY;
                break;
            case 1:
                c->action = EXCHANGE;
                break;
            case 2:
                c->action = CANCEL;
                break;
            case 3:
                c->action = REFUND;
                break;
            case 4:
                c->action = RESERVE;
                break;
        }

        c->screening_id = rand() % cinema->num_screenings;
        c->seat_id = 0;

        c->ticket_id = rand() % 10; // pour les tests
        c->new_screening_id = rand() % cinema->num_screenings;
        c->new_seat_id = rand() % cinema->screenings[c->new_screening_id]->room->capacity;

        Client* snapshot = malloc(sizeof(Client));
        *snapshot = *c;

        enqueue_client(cinema->client_queue, snapshot);


        printf("[CLIENT %d] Action %s envoyée\n", c->id, action_to_string(c->action));

        sleep(4); // temps de réflexion
    }

    printf("[CLIENT %d] Fin des actions\n", c->id);
    return NULL;
}

//client avec réservation
void* client_thread2(void* arg) {
    Cinema* cinema = (Cinema*)arg;

    Client* c = malloc(sizeof(Client));
    if (!c) return NULL;

    c->id = 100+rand() % 100;
    c->age = 10+ rand() % 20;
    c->has_reservation = 1;

    snprintf(c->name, sizeof(c->name), "Client_%d", c->id);
    snprintf(c->email, sizeof(c->email), "client%d@mail.com", c->id);

    int actions = 1 + rand() % 2; // 1 à 3 actions

    for (int i = 0; i < actions; i++) {

        int choice = rand() % 4;

        switch (choice) {
            case 0:
                c->action = RESERVE;
                break;
            case 1:
                c->action = VALIDATE_RESERVATION;
                break;
            case 2:
                c->action = MODIFY_RESERVATION;
                break;
            case 3:
                c->action = CANCEL_RESERVATION;
                break;
        }

        c->screening_id = rand() % cinema->num_screenings;
        c->seat_id = rand() % cinema->screenings[c->screening_id]->room->capacity;

        c->ticket_id = rand() % 10; // pour les tests
        c->new_screening_id = rand() % cinema->num_screenings;
        c->new_seat_id = rand() % cinema->screenings[c->new_screening_id]->room->capacity;

        Client* snapshot = malloc(sizeof(Client));
        *snapshot = *c;

        enqueue_client(cinema->client_queue, snapshot);

        printf("[CLIENT %d] Action %s envoyée\n", c->id, action_to_string(c->action));

        sleep(3); // temps de réflexion
    }

    printf("[CLIENT %d] Fin des actions\n", c->id);
    return NULL;
}
