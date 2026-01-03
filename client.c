#include <stdlib.h>
#include "client.h"

// creation de la file d'attente des clients
ClientQueue* clientqueue_create() {
    ClientQueue* queue = (ClientQueue*)malloc(sizeof(ClientQueue));
    if (!queue) return NULL;
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    return queue;
}

// ajouter un client
void enqueue_client(ClientQueue* queue, Client* client) {
    ClientNode* new_node = (ClientNode*)malloc(sizeof(ClientNode));
    if (!new_node) return;
    new_node->client = client;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->tail) {
        queue->tail->next = new_node;
        queue->tail = new_node;
    } else {
        queue->head = new_node;
        queue->tail = new_node;
    }
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

// retirer un client
Client* dequeue_client(ClientQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    while (!queue->head) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    ClientNode* temp = queue->head;
    queue->head = temp->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    pthread_mutex_unlock(&queue->mutex);
    Client* client = temp->client;
    free(temp);
    return client;
}
