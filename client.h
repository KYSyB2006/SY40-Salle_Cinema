#ifndef CLIENT_H
#define CLIENT_H
#include <pthread.h>
#include "struct.h"


typedef enum {
              BUY,
              RESERVE,
              CANCEL_RESERVATION,
              MODIFY_RESERVATION,
              VALIDATE_RESERVATION,
              EXCHANGE,
              CANCEL,
              REFUND
            } ClientAction;

typedef struct {
                int id;
                char name[50];
                char email[50];
                int age;
                int has_reservation;     // priorité
                ClientAction action;
                int screening_id;
                int seat_id;
                int new_seat_id;  
                int new_screening_id; 
                int ticket_id;  
              } Client;    
              
typedef struct ClientNode {
                Client* client;
                struct ClientNode* next;
            } ClientNode;

typedef struct {
                ClientNode* head;
                ClientNode* tail;
                pthread_mutex_t mutex;
                pthread_cond_t cond;
            } ClientQueue;

ClientQueue* clientqueue_create();
void clientqueue_destroy(ClientQueue* queue);

void enqueue_client(ClientQueue* queue, Client* client);
Client* dequeue_client(ClientQueue* queue);


#endif //CLIENT_H