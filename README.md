# 🎬 SY40-Salle_Cinema - Système de Gestion de Cinéma Multi-Thread

## 📋 Table des matières
1. [Vision du Projet](#vision-du-projet)
2. [Architecture Générale](#architecture-générale)
3. [Système de Threads](#système-de-threads)
4. [Mécanisme de Synchronisation](#mécanisme-de-synchronisation)
5. [Système de Priorités](#système-de-priorités)
6. [Documentation des Fichiers](#documentation-des-fichiers)

---

## 🎯 Vision du Projet

### Concept Fondamental

**SY40-Salle_Cinema** est un système de simulation et de gestion de cinéma à multi-salles conçu pour démontrer les principes avancés de la programmation concurrente en C. Le projet modélise un cinéma réel où plusieurs clients interagissent simultanément avec le système pour effectuer des opérations (achats, réservations, échanges, annulations, remboursements).

### Objectifs Clés

1. **Concurrence Réaliste** : Simuler les interactions simultanées de clients dans un cinéma moderne
2. **Gestion d'État Cohérente** : Maintenir la cohérence des données partagées (places, billets, statuts) lors d'accès concurrents
3. **Flux de Traitement Équitable** : Implémenter un système de files d'attente qui respecte les priorités (clients avec réservations vs clients standards)
4. **Alternatives Dynamiques** : Lorsqu'une transaction échoue, proposer automatiquement des alternatives (siège alternatif, séance alternative, film alternatif)
5. **Traçabilité Complète** : Enregistrer toutes les transactions avec leurs statuts et raisons d'échec

### Scénario Utilisateur Type

```
Client arrive au cinéma → Enqueue dans la file d'attente
    ↓
Hostess/Kiosk extrait le client → Crée une intention de transaction
    ↓
Processor traite l'intention → Valide l'opération
    ↓
Si succès → Transaction complétée
Si échec → Calcul d'alternatives → Proposées au client
```

---

## 🏗️ Architecture Générale

### Composants Principaux

```
┌─────────────────────────────────────────────────────────┐
│                     CINEMA                              │
├─────────────────────────────────────────────────────────┤
│  • Salles (Rooms) - Contiennent les sièges             │
│  • Films (Movies) - Catalogues des films               │
│  • Séances (Screenings) - Projections horaires         │
│  • Billets (Tickets) - Transactions clients            │
│  • Files d'attente (Queues) - Entrées clients          │
└─────────────────────────────────────────────────────────┘
        ↓
┌─────────────────────────────────────────────────────────┐
│            SYSTEME DE TRAITEMENT CONCURRENT             │
├─────────────────────────────────────────────────────────┤
│  Threads d'Entrée:                                      │
│    • Clients simples (CLIENT_THREAD)                    │
│    • Clients avec réservations (CLIENT_THREAD2)         │
│                ↓                                        │
│  Threads de Traitement:                                 │
│    • Hostess (file comptoir)                           │
│    • Kiosk (file borne)                                │
│                ↓                                        │
│  Thread de Processeur Central:                          │
│    • Exécute les intentions                             │
│    • Valide les opérations                              │
│    • Calcule les alternatives                           │
└─────────────────────────────────────────────────────────┘
```

### Types de Données Clés

#### **Enumerations (struct.h)**

```c
SeatStatus     : SEAT_AVAILABLE, SEAT_RESERVED, SEAT_SOLD
TicketStatus   : TICKET_VALID, TICKET_SOLD, TICKET_CANCELLED, TICKET_REFUNDED, TICKET_EXCHANGED
AgeRating      : AGE_ALL, AGE_12, AGE_16, AGE_18
ClientAction   : BUY, RESERVE, CANCEL_RESERVATION, MODIFY_RESERVATION, 
                 VALIDATE_RESERVATION, EXCHANGE, CANCEL, REFUND
```

#### **Structures Principales**

- **Room** : Représente une salle de cinéma avec ses sièges (seats), dimensions, et capacité
- **Movie** : Film avec titre, durée, note d'âge, genre
- **Screening** : Séance spécifique (film + salle + horaire + prix)
- **Seat** : Siège individuel avec statut et ID de billet associé
- **Ticket** : Billet de client avec informations personnelles, statut, et métadonnées de transaction
- **TicketList** : Liste doublement chaînée de billets avec mutex pour synchronisation
- **Cinema** : Structure englobante contenant toutes les ressources du cinéma

---

## 🔄 Système de Threads

### Architecture Multi-Thread

Le système utilise **5 types de threads** travaillant en concert :

### 1. **CLIENT THREADS** (Producteurs)
**Fichiers** : [threads.c](threads.c#L214-L270), [threads.h](threads.h#L26)

**Responsabilité** : Simuler les clients réels qui arrivent au cinéma et soumettent des demandes.

**Implémentations** :
- `client_thread()` : Client standard qui peut acheter, réserver, annuler, échanger ou se faire rembourser
- `client_thread2()` : Client avec réservation active qui peut modifier, valider, ou annuler sa réservation

**Processus** :
1. Génère un client avec identité aléatoire (nom, email, âge, ID)
2. Sélectionne une action aléatoire parmi celles disponibles
3. Choisit aléatoirement une séance et un siège
4. Enqueued le client dans la `ClientQueue` centrale
5. Attend quelques secondes avant de recommencer (simulation du temps de réflexion)

**Caractéristique importante** : Les threads clients s'exécutent dans une boucle infinie, générant continuellement du trafic.

### 2. **HOSTESS THREAD** (Producteur d'Intentions - Comptoir)
**Fichiers** : [threads.c](threads.c#L63-L89), [threads.h](threads.h#L24)

**Responsabilité** : Représente les agents au comptoir du cinéma. Extrait les clients de la file d'attente et crée des intentions de transaction.

**Processus** :
1. Déqueue un client de `cinema->client_queue`
2. Convertit le client en `TicketIntention` (structure intermédiaire contenant tous les paramètres)
3. Enqueue l'intention dans `cinema->counter_list` (file des intentions du comptoir)
4. Libère la mémoire du client
5. Attend 100ms avant de traiter le suivant

**Raison de cette architecture** : Découpler la soumission des demandes de leur traitement.

### 3. **KIOSK THREAD** (Producteur d'Intentions - Borne)
**Fichiers** : [threads.c](threads.c#L91-L117), [threads.h](threads.h#L25)

**Responsabilité** : Représente les bornes de vente automatiques du cinéma. Fonctionne comme hostess mais avec sa propre file.

**Différence avec Hostess** : Traite les mêmes demandes mais via une file d'attente différente (simulation de 2 points de vente parallèles).

**Processus** : Identique à hostess, mais enqueue dans `cinema->kiosk_list` au lieu de `cinema->counter_list`.

### 4. **PROCESSOR THREAD** (Consommateur Principal)
**Fichiers** : [threads.c](threads.c#L119-L237), [threads.h](threads.h#L27)

**Responsabilité** : **Cœur du système**. Traite les intentions de transaction et exécute les opérations.

**Processus Détaillé** :

```c
Boucle infinie:
  1. Alterne entre counter_list et kiosk_list (équité)
  2. Pop une intention
  3. Selon l'action:
     
     BUY / EXCHANGE:
       - Boucle avec retry jusqu'à succès ou alternatives épuisées
       - Appelle purchase_ticket() ou exchange_ticket()
       - Si échec (siège pris, âge insuffisant) → Calcule alternatives
       - Tente avec les alternatives proposées
     
     RESERVE / MODIFY / VALIDATE:
       - Appelle les services de réservation
       - Enregistre dans la base de données
     
     CANCEL / REFUND:
       - Appelle cancel_ticket() ou refund_ticket()
       - Libère les ressources
       - Enregistre la transaction

  4. Affiche le résultat et l'intention traitée
  5. Attend 100ms avant de continuer
```

**Caractéristique clé** : Variable `lastqueue` maintient l'alternance entre les deux files pour assurer l'équité.

### 5. **CLIENT THREAD (2 Variantes)**
Déjà décrites ci-dessus. Génèrent le trafic en continu.

### Schéma de Flux des Threads

```
CLIENT_THREAD (génère)        CLIENT_THREAD2 (génère)
        ↓                              ↓
  ┌──────────────────────────────────────┐
  │    CLIENTQUEUE (file d'attente)      │
  │  [Mutex + Condition Variable]        │
  └──────────────────────────────────────┘
        ↓                              ↓
   HOSTESS (consomme)          KIOSK (consomme)
   Transforme en Intention     Transforme en Intention
        ↓                              ↓
┌─────────────────────┐    ┌─────────────────────┐
│  COUNTER_LIST       │    │   KIOSK_LIST        │
│ [Mutex + Priorité]  │    │ [Mutex + Priorité]  │
└─────────────────────┘    └─────────────────────┘
        ↑                              ↑
        └──────────────┬──────────────┘
                       ↓
              PROCESSOR_THREAD
              (alterne entre les deux)
              Exécute les intentions
              Calcule les alternatives
                       ↓
             BASE DE DONNÉES CINEMA
             (Tickets, Seats, etc.)
```

---

## 🔐 Mécanisme de Synchronisation

### Problème Adressé

Sans synchronisation, les accès concurrents causeraient :
- **Race Conditions** : Deux threads achètent le même siège simultanément
- **Data Corruption** : Modifications partielles interrompues par d'autres threads
- **Deadlocks** : Threads attendant mutuellement

### Solutions Implémentées

### 1. **Mutex (Mutual Exclusion)**
Utilisé pour protéger les sections critiques où les données partagées sont modifiées.

#### Emplacements Clés :

```c
// TicketList - Protection de la liste des billets
typedef struct {
    TicketNode* head;
    TicketNode* actual;
    TicketNode* tail;
    int size;
    pthread_mutex_t mutex;  // ← Protège les opérations CRUD
} TicketList;

// TicketIntentionList - Protection des intentions
typedef struct {
    TicketIntentionNode* head;
    TicketIntentionNode* tail;
    int size;
    pthread_mutex_t mutex;  // ← Protège push/pop
} TicketIntentionList;

// ClientQueue - Protection de la file de clients
typedef struct {
    ClientNode* head;
    ClientNode* tail;
    pthread_mutex_t mutex;      // ← Protège enqueue/dequeue
    pthread_cond_t cond;        // ← Signal quand queue non-vide
} ClientQueue;
```

#### Patterns d'Utilisation :

**Pattern 1 : Lock Simple**
```c
pthread_mutex_lock(&list->mutex);
// Code critique ici
pthread_mutex_unlock(&list->mutex);
```

**Pattern 2 : Lock dans push_intention** (threads.c, ligne 23-36)
```c
pthread_mutex_lock(&list->mutex);
// Insérer en tête si priorité, sinon en queue
if (t->has_reservation && list->head) {
    node->next = list->head;
    list->head = node;
} else {
    // Ajouter en queue normalement
}
list->size++;
pthread_mutex_unlock(&list->mutex);
```

**Pattern 3 : Lock dans pop_intention** (threads.c, ligne 38-54)
```c
pthread_mutex_lock(&list->mutex);
if (!list->head) {
    pthread_mutex_unlock(&list->mutex);
    return NULL;
}
// Extraire et libérer le nœud
pthread_mutex_unlock(&list->mutex);
```

### 2. **Condition Variables**
Utilisées pour synchroniser les threads en attente avec les événements.

**Localisation** : [client.h](client.h#L19-L20)

```c
typedef struct {
    ClientNode* head;
    ClientNode* tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;  // ← Signal que queue a reçu des clients
} ClientQueue;
```

**Cas d'Usage** : Les threads consumer (hostess, kiosk) peuvent attendre que la file se remplisse plutôt que de faire du polling constant.

### 3. **Sérialisation des Opérations Critiques**

Les opérations qui modifient l'état du cinéma (sièges, billets) sont sérialisées via le **PROCESSOR_THREAD unique** :

```
// Deux threads ne peuvent JAMAIS exécuter purchase_ticket() 
// simultanément pour le même screening car il n'y a qu'UN processor
PROCESSOR_THREAD → purchase_ticket() → Modifie seats atomiquement
PROCESSOR_THREAD → exchange_ticket() → Modifie seats + tickets atomiquement
```

### 4. **Atomicité au Niveau Structures**

Les structures critiques sont toujours modifiées :
- Via des fonctions dédiées (purchase_ticket, cancel_ticket)
- Sous protection mutex
- En une seule opération non-interruptible

### Garantie d'atomicité des transactions (reservation_service.c & ticket_service.c)

L'atomicité des opérations critiques (achat, réservation, échange, modification, validation) est garantie par des mutex déclarés dans chaque module service :

- **Mutex de `ticket_service.c`** : Deux mutex statiques
  - `static pthread_mutex_t ticket_mutex` : Protège les opérations de lecture/écriture sur la liste des billets (`cinema->tickets`) et les métadonnées associées (comptage, statistiques).
  - `static pthread_mutex_t seat_mutex` : Protège les accès concurrents aux statuts des sièges (marquage d'un siège comme `SEAT_SOLD`, `SEAT_AVAILABLE`, etc.) et leur association avec les billets.

- **Mutex de `reservation_service.c`** : Mêmes deux mutex
  - `static pthread_mutex_t seat_mutex` : Protège les modifications de sièges lors de réservations (transition vers `SEAT_RESERVED`).
  - `static pthread_mutex_t ticket_mutex` : Implicite pour la cohérence des réservations.
  - **Mutex partagé** : `cinema->reservation_list->mutex` protège la liste des réservations en attente lors de l'ajout ou la modification d'une réservation.

- **Pattern d'acquisition dans `purchase_ticket()`** :
  ```c
  pthread_mutex_lock(&ticket_mutex);
  pthread_mutex_lock(&seat_mutex);
  // Vérifications (âge, disponibilité du siège)
  // Création du ticket, mise à jour du siège
  // Mise à jour cinema->tickets
  pthread_mutex_unlock(&seat_mutex);
  pthread_mutex_unlock(&ticket_mutex);
  ```
  Cette séquence ordonnée (d'abord `ticket_mutex`, puis `seat_mutex`) assure une atomicité de la transaction sans deadlock.

- **Pattern d'acquisition dans `exchange_ticket()`** : Même ordre de locks pour rechercher le billet existant, libérer l'ancien siège et acquérir le nouveau siège.

- **Pattern dans `make_reservation()`** : Acquiert `seat_mutex` pour réserver un siège, puis relâche et acquiert `cinema->reservation_list->mutex` pour ajouter la réservation à la liste partagée (déverrouillage intermédiaire pour réduire la contention).

- **Pattern dans `modify_reservation()`** : Déverrouille `cinema->reservation_list->mutex` après localisation de la réservation, puis acquiert `seat_mutex` pour effectuer les modifications de sièges, afin de minimiser la durée du premier verrou.

- **Validation et cohérence** : Toute transition d'état (ex. `SEAT_AVAILABLE → SEAT_SOLD` ou `TICKET_VALID → TICKET_SOLD`) se fait intégralement sous les locks, sans opération partiellement appliquée visible aux autres threads.

En résumé, les deux mutex (`ticket_mutex` et `seat_mutex`) encadrent chaque opération métier et assurent que les modifications du statut des sièges et des billets sont atomiques et cohérentes.

---

## ⭐ Système de Priorités

### Concept

Certains clients (ceux avec des réservations) doivent être traités en priorité par rapport aux clients standards. Ceci est implémenté à deux niveaux :

### Niveau 1 : Priorité dans les Files d'Intentions

**Implémentation** : [threads.c](threads.c#L23-L36)

```c
void push_intention(TicketIntentionList* list, TicketIntention* t) {
    // ...
    // priorité aux réservations
    if (t->has_reservation && list->head) {
        // Insérer au DEBUT de la file
        node->next = list->head;
        list->head = node;
    } else {
        // Insérer à la FIN de la file (FIFO normal)
        if (!list->tail) list->head = list->tail = node;
        else {
            list->tail->next = node;
            list->tail = node;
        }
    }
}
```

**Résultat** : Un client avec `has_reservation = 1` saute devant la file, mais seulement s'il existe déjà des clients en attente.

### Niveau 2 : Traitement Équitable entre Files

**Implémentation** : [threads.c](threads.c#L119-L237)

```c
static int lastqueue = 0;
while (1) {
    if (lastqueue == 0) {
        t = pop_intention(cinema->counter_list);
        if (!t) t = pop_intention(cinema->kiosk_list);
    } else {
        t = pop_intention(cinema->kiosk_list);
        if (!t) t = pop_intention(cinema->counter_list);
    }
    lastqueue = !lastqueue;  // Alterne à chaque itération
```

**Raison** : Assurer qu'aucune file (comptoir ou borne) ne monopolise le processor.

### Niveau 3 : Actions Prioritaires

Actions liées aux réservations existantes ont priorité implicite :
- `VALIDATE_RESERVATION` : Client finalise sa réservation
- `MODIFY_RESERVATION` : Client change sa réservation
- `CANCEL_RESERVATION` : Client annule sa réservation

Ces actions partent souvent de `client_thread2` qui a `has_reservation = 1`.

### Scénario Illustratif

```
État initial:
Counter Queue:  [Client A (no reservation)] → [Client B (no reservation)]
Kiosk Queue:    [Client C (WITH reservation)]

Processor traite:
1. Pop de Counter (lastqueue=0) → Traite Client A
2. Pop de Kiosk (lastqueue=1) → Traite Client C ← PRIORITAIRE
3. Pop de Counter (lastqueue=0) → Traite Client B
4. Pop de Kiosk (lastqueue=1) → File vide, retry Counter
```

---

## 📁 Documentation des Fichiers

### 1. [main.c](main.c)
**Taille** : Minimale (commentaire uniquement)
**Rôle** : Point d'entrée du projet (en construction/test)
**Contenu** : `//Gestion et simulation de notre cinema`

### 2. [struct.h](struct.h)
**Taille** : 131 lignes
**Rôle** : **Cœur des définitions de données**
**Contenu Détaillé** :

#### Énumérations
- `SeatStatus` : État d'un siège (disponible, réservé, vendu)
- `TicketStatus` : État d'un billet (valide, vendu, annulé, remboursé, échangé)
- `AgeRating` : Classification d'âge des films (Tous publics, 12+, 16+, 18+)

#### Structures de Base
- **Seat** : Identité (id, row, col), statut, ticket_id lié
- **Movie** : Identité, titre, durée, note d'âge, genre
- **Room** : Identification, tableau dynamique de sièges 2D, capacité
- **Screening** : Liaison film-salle, horaire, prix, compteurs (seats_sold, seats_reserved)
- **EventReservation** : Réservation d'événement (pas film, but événement privé)
- **Ticket** : Information client, statut du billet, timestamps (achat, réservation)
- **TicketNode** & **TicketList** : Liste chaînée de billets avec synchronisation
- **CinemaStatistics** : Compteurs agrégés (total vendus, réservés, etc.)

#### Structure Englobante
```c
// Supposée définie quelque part (peut-être dans gestion.h)
Cinema: 
  - Tableau de rooms
  - Tableau de movies  
  - Tableau de screenings
  - TicketList
  - ClientQueue
  - TicketIntentionList counter_list
  - TicketIntentionList kiosk_list
```

### 3. [client.h](client.h) & [client.c](client.c)
**Responsabilité** : Modélisation et gestion des clients

#### client.h (Déclarations)
```c
ClientAction : Enum des 8 actions possibles
  - BUY : Acheter un billet directement
  - RESERVE : Faire une réservation
  - CANCEL_RESERVATION : Annuler réservation
  - MODIFY_RESERVATION : Changer réservation
  - VALIDATE_RESERVATION : Valider réservation → achat
  - EXCHANGE : Échanger billet existant
  - CANCEL : Annuler achat (mais pas réservation)
  - REFUND : Remboursement d'achat

Client : Structure représentant un client unique
  - id : Identifiant unique
  - name, email, age : Données personnelles
  - has_reservation : Flag de priorité (0 ou 1)
  - action : Action demandée
  - screening_id, seat_id : Ressource demandée
  - ticket_id : Pour modifications/annulations
  - new_seat_id, new_screening_id : Pour modifications

ClientQueue : File d'attente avec mutex + condition variable
  - Utilisée pour accumuler les clients avant traitement
```

#### client.c (Implémentation)
- `clientqueue_create()` : Alloue et initialise une queue
- `clientqueue_destroy()` : Libère la queue
- `enqueue_client()` : Ajoute un client (thread-safe)
- `dequeue_client()` : Retire un client (thread-safe)
- `action_to_string()` : Convertit action enum en texte pour logs

### 4. [threads.h](threads.h) & [threads.c](threads.c)
**Responsabilité** : **Orchestration du système concurrentiel**

#### threads.h (Déclarations)
```c
TicketIntention : Structure intermédiaire
  - Copie tous les paramètres du Client
  - Ajoute has_reservation pour priorité
  - Utilisée entre hostess/kiosk et processor

TicketIntentionList : File avec priorité
  - Mutex pour synchronisation
  - head/tail pour gestion FIFO
  - Peut réordonner basé sur has_reservation

Fonctions déclarées:
  - ticketlistint_create() : Crée une intention list
  - push_intention() : Ajoute avec priorité
  - pop_intention() : Extrait (FIFO pour même priorité)
  - hostess_thread() : Thread comptoir
  - kiosk_thread() : Thread borne
  - processor_thread() : Thread central
  - client_thread() : Thread client standard
  - client_thread2() : Thread client réservation
```

#### threads.c (Implémentation)

**Détail des Fonctions Principales** :

##### `push_intention()` (lignes 23-43)
- Crée un nœud pour l'intention
- Verrouille le mutex
- **Logique de Priorité** : Si client a réservation ET liste non-vide → insérer en tête
- Sinon : insérer en queue (FIFO)
- Incrémente la taille
- Déverrouille

##### `pop_intention()` (lignes 45-59)
- Verrouille le mutex
- Vérifie que la liste n'est pas vide
- Extrait le premier nœud
- Met à jour head/tail si nécessaire
- Décrémente la taille
- Déverrouille et retourne l'intention

##### `hostess_thread()` (lignes 63-89)
- Boucle infinie
- Dequeue un client de cinema->client_queue
- Crée une TicketIntention et copie tous les champs
- Push dans cinema->counter_list
- Affiche log
- Attend 100ms
- Recommence

##### `kiosk_thread()` (lignes 91-117)
- Identique à hostess, mais push dans cinema->kiosk_list

##### `processor_thread()` (lignes 119-237)
**Le cœur du système** :

1. **Boucle Principale** :
   ```c
   while (1) {
       // Alterne entre files
       if (lastqueue == 0) {
           t = pop_intention(cinema->counter_list);
           if (!t) t = pop_intention(cinema->kiosk_list);
       } else {
           t = pop_intention(cinema->kiosk_list);
           if (!t) t = pop_intention(cinema->counter_list);
       }
       lastqueue = !lastqueue;
   ```

2. **Traitement selon Action** :
   - **BUY / EXCHANGE** : Boucle de retry avec alternatives
     - Appelle `purchase_ticket()` ou `exchange_ticket()`
     - Si `SEAT_UNAVAILABLE` ou `AGE_DENIED` → calcule alternatives
     - Tente avec alternatives jusqu'à succès ou épuisement
   
   - **RESERVE** : Appelle `make_reservation()`
   - **MODIFY_RESERVATION** : Appelle `modify_reservation()`
   - **VALIDATE_RESERVATION** : Appelle `validate_reservation()`
   - **CANCEL_RESERVATION** : Appelle `cancel_reservation()`
   - **CANCEL** : Appelle `cancel_ticket()`
   - **REFUND** : Appelle `refund_ticket()`

##### `client_thread()` (lignes 239-287)
- Crée un client standard (has_reservation = 0)
- Boucle infinie générant 1-4 actions aléatoires
- Sélectionne aléatoirement : BUY, EXCHANGE, CANCEL, REFUND
- Choisit screening et siège aléatoires
- Enqueue dans cinema->client_queue
- Attend 4 secondes entre actions

##### `client_thread2()` (lignes 289-337)
- Crée un client avec réservation (has_reservation = 1)
- Boucle infinie générant 1-2 actions aléatoires
- Sélectionne aléatoirement : RESERVE, VALIDATE_RESERVATION, MODIFY_RESERVATION, CANCEL_RESERVATION
- Enqueue dans cinema->client_queue
- Attend 3 secondes entre actions

### 5. [ticket_service.h](ticket_service.h) & [ticket_service.c](ticket_service.c)
**Responsabilité** : Logique métier des ventes et échanges de billets

#### ticket_service.h
```c
TicketResult enum:
  - OK : Transaction réussie
  - INVALID : Données invalides
  - SEAT_UNAVAILABLE : Siège déjà vendu/réservé
  - AGE_DENIED : Client trop jeune pour le film

Fonctions:
  - purchase_ticket() : Achète un billet pour une séance/siège
  - exchange_ticket() : Échange un billet existant pour un autre
  - cancel_ticket() : Annule un achat
  - refund_ticket() : Effectue remboursement
  - verify_age() : Vérifie que l'âge client ≥ âge minimum du film
```

#### ticket_service.c (Logique)
- **purchase_ticket()** :
  - Vérifie l'âge (AGE_DENIED si trop jeune)
  - Vérifie le siège (SEAT_UNAVAILABLE si pris)
  - Crée un nouveau Ticket avec status TICKET_SOLD
  - Marque le siège comme SEAT_SOLD
  - Ajoute à cinema->tickets
  - Retourne OK
  - Peut retourner liste d'alternatives via paramètre pointer `AlternativeList**`

- **exchange_ticket()** :
  - Trouve le billet existant
  - Annule l'ancien (libère siège)
  - Tente d'acheter au nouveau siège
  - Si échec → alternatives proposées
  - Si succès → Ticket status = TICKET_EXCHANGED

- **cancel_ticket()** & **refund_ticket()** :
  - Trouvent le billet
  - Libèrent le siège associé
  - Changent status
  - Retournent 1 (succès) ou 0 (échec)

### 6. [reservation_service.h](reservation_service.h) & [reservation_service.c](reservation_service.c)
**Responsabilité** : Logique métier des réservations

#### reservation_service.h
```c
Fonctions:
  - make_reservation() : Crée une réservation (billet status TICKET_VALID)
  - modify_reservation() : Change siège/séance de réservation
  - validate_reservation() : Convertit réservation → achat (TICKET_VALID → TICKET_SOLD)
  - cancel_reservation() : Annule réservation et libère ressources
```

#### Flux Réservation
```
Client fait réservation
  → Ticket status = TICKET_VALID
  → Siège status = SEAT_RESERVED
  → Pas d'argent payé
         ↓
Client modifie (optionnel)
  → Libère ancien siège
  → Réserve nouveau siège
         ↓
Client valide
  → Ticket status = TICKET_SOLD
  → Siège status = SEAT_SOLD
  → Argent payé
         ↓
OU Client annule
  → Ticket status = TICKET_CANCELLED
  → Siège status = SEAT_AVAILABLE
```

### 7. [alternatives.h](alternatives.h) & [alternatives.c](alternatives.c)
**Responsabilité** : Génération de solutions de remplacement en cas d'échec

#### alternatives.h
```c
AlternativeChoice : Paire (Screening, Seat)

AlternativeType enum:
  - ALT_CHANGE_SEAT : Offrir autre siège même séance
  - ALT_CHANGE_SCREENING : Offrir autre séance même film
  - ALT_CHANGE_MOVIE : Offrir autre film entièrement

AlternativeOption : Une option (type, screening_id, seat_id)

AlternativeList : Tableau d'options + count

Fonctions clés:
  - list_available_seats() : Récupère sièges libres d'une salle
  - list_screenings_same_movie() : Séances du même film
  - list_screenings_by_age() : Séances compatibles avec âge client
  - compute_alternatives() : Génère liste complète d'alternatives
```

#### compute_alternatives() Logic
Quand client demande siège indisponible :
```
1. Siège demandé n'existe pas ou pris
   → Chercher siège libre dans même séance (ALT_CHANGE_SEAT)
   → Chercher séance du même film avec siège (ALT_CHANGE_SCREENING)
   → Chercher autre film dans même salle avec siège (ALT_CHANGE_MOVIE)

2. Âge insuffisant pour film
   → Chercher films compatibles (note d'âge ≤ âge client)
   → Proposer séances avec sièges disponibles

3. Retourner AlternativeList avec toutes les options viables
```

### 8. [gestion.h](gestion.h) & [gestion.c](gestion.c)
**Responsabilité** : Gestion globale du cinéma et statistiques

#### gestion.h - Déclarations Principales
```c
StatisticsThreadArgs : Arguments pour thread statistiques
  - cinema : Pointeur vers cinéma
  - result : Où stocker résultat statistiques
  - mutex : Protection accès résultat
  - status : Code retour

Cinema structure (probablement) :
  - Tableau rooms, movies, screenings
  - TicketList
  - ClientQueue  
  - TicketIntentionList counter_list et kiosk_list
  - Compteurs divers

Fonctions principales:
  - cinema_create() : Alloue structure cinéma
  - room_create(), movie_create(), screening_create()
  - cinema_add_room(), cinema_add_movie(), cinema_add_screening()
  - remove_* : Suppression
  - list_* : Lister les ressources
  - calculate_occupancy() : Pourcentage sièges occupés
  - lock_screening() / unlock_screening() : Mutex pour séances
  - can_switch_film() : Vérifier si film peut changer (< 20% vendus)
  - switch_film() : Changer le film d'une séance
  - update_dynamic_schedule() : Ajuster programmation
  - generate_cinemastatistics() : Agréger stats
  - statistics_thread() : Thread collecteur stats
  - liberation_room_after_event() : Nettoyer après événement
  - liberation_places_at_end_screening() : Libérer sièges après séance
  - event_reservation_create() : Créer réservation privée
```

### 9. [Makefile](Makefile)
**Rôle** : Automatisation de la compilation

**Contenu** :
```makefile
CFLAGS = -Wall -Wextra  # Activer tous les avertissements

EXEC2 = ticket_test  # Exécutable cible
EXEC2_SRCS = client.c test_gestion_ticket.c gestion.c alternatives.c \
             ticket_service.c reservation_service.c threads.c

all: $(EXES)  # Règle par défaut

$(EXEC2): compile les sources avec gcc
clean: rm -f $(EXES)  # Supprime les exécutables
```

**Utilisation** :
```bash
make          # Compile ticket_test
make clean    # Supprime les exécutables
```

### 10. [test_gestion_cinema.c](test_gestion_cinema.c) & [test_gestion_ticket.c](test_gestion_ticket.c)
**Rôle** : Tests unitaires et d'intégration

#### test_gestion_cinema.c
Teste les fonctions de gestion du cinéma :
- Création/suppression de salles, films, séances
- Listes et calculs d'occupation

#### test_gestion_ticket.c
Teste les opérations de billets :
- Achat, échange, annulation, remboursement
- Simulation concurrente avec threads multiples
- Vérification de la cohérence des états

---

## 🔄 Flux Complet d'une Transaction

### Exemple : Client achète un billet

```
ÉTAPE 1 : Génération
  Client thread exécute client_thread()
  └─→ Génère Client{id=5, action=BUY, screening_id=2, seat_id=3, age=25}
  └─→ enqueue_client(cinema->client_queue, client)

ÉTAPE 2 : Transformation  
  Hostess thread s'exécute
  └─→ c = dequeue_client(cinema->client_queue)
  └─→ Crée TicketIntention{...same data..., has_reservation=0}
  └─→ push_intention(cinema->counter_list, intention)

ÉTAPE 3 : Traitement
  Processor thread s'exécute
  └─→ lastqueue = 0 → pop_intention(cinema->counter_list)
  └─→ Récupère intention
  └─→ action == BUY → Appelle purchase_ticket()
        
        purchase_ticket():
          - verify_age(movie, 25) → OK (film pour tous publics)
          - screening->room->seats[3]->status == SEAT_AVAILABLE → OK
          - Crée Ticket{status=TICKET_SOLD, purchase_time=now}
          - seats[3]->status = SEAT_SOLD
          - seats[3]->ticket_id = ticket.id
          - Ajoute à cinema->tickets list
          - Retourne OK
  
  └─→ Affiche résultat
  └─→ Attend 100ms
  └─→ Retour début boucle

RÉSULTAT FINAL:
  ✓ Billet créé et dans base de données
  ✓ Siège marqué comme vendu
  ✓ Client notifié (implicitement via logs)
```

### Exemple : Échec et Alternatives

```
Client demande siège 3 de séance 2, mais celui-ci est pris

ÉTAPE 1-2 : Identique

ÉTAPE 3 : Traitement avec Alternatives
  processor_thread() → purchase_ticket()
  
  purchase_ticket():
    - verify_age() → OK
    - seats[3] == SEAT_SOLD → SEAT_UNAVAILABLE retourné!
    - Aucune alternative calculée dans purchase_ticket directement
  
  processor_thread() continue:
    - result == SEAT_UNAVAILABLE → Entre dans boucle retry
    - alt = compute_alternatives(cinema, ticket, "SEAT_UNAVAILABLE")
    
    compute_alternatives():
      - Cherche siège libre en séance 2 → Trouve siège 5!
      - Cherche autre séance même film → Trouve séance 3!
      - Crée AlternativeList[
          {type:ALT_CHANGE_SEAT, screening_id:2, seat_id:5},
          {type:ALT_CHANGE_SCREENING, screening_id:3, seat_id:...}
        ]
    
    - Tente alternative 1 : purchase_ticket(screening 2, seat 5)
    - Succès! → Retourne OK
    - Affiche: "[PROCESSOR] Client 5 -> BUY via SEAT ALTERNATIVE"

RÉSULTAT FINAL:
  ✓ Billet créé pour siège 5 (alternative)
  ✓ Client satisfait avec proposition alternative
  ✓ Ressource allouée de manière efficace
```

---

## 🚀 Démarrage du Système

### Initialisation
```c
// Dans main (appelé une fois):
Cinema* cinema = cinema_create(num_rooms);
cinema_add_room(cinema, "Salle 1", 5, 10);
cinema_add_movie(cinema, "Film 1", 120, AGE_12, "Action");
cinema_add_screening(...);

ClientQueue* queue = clientqueue_create();
TicketIntentionList* counter = ticketlistint_create();
TicketIntentionList* kiosk = ticketlistint_create();
```

### Lancement Threads
```c
pthread_t hostess, kiosk, processor;
pthread_t clients[NUM_CLIENTS];

pthread_create(&hostess, NULL, hostess_thread, cinema);
pthread_create(&kiosk, NULL, kiosk_thread, cinema);
pthread_create(&processor, NULL, processor_thread, cinema);

for (int i = 0; i < NUM_CLIENTS; i++) {
    pthread_create(&clients[i], NULL, (i % 2 == 0) ? 
                   client_thread : client_thread2, cinema);
}

// Attendre threads (normalement infini...)
pthread_join(hostess, NULL);
// ...
```

### Output Typique
```
[CLIENT 45] Action BUY envoyée
[HOSTESS] Client 45 -> intention ajoutée
[PROCESSOR] Client 45 -> BUY OK (siège acquis)
[CLIENT 120] Action RESERVE envoyée
[KIOSK] Client 120 -> intention ajoutée
[PROCESSOR] Client 120 -> RESERVE OK
```

---

## 📊 Avantages de cette Architecture

| Aspect | Bénéfice |
|--------|----------|
| **Séparation des Responsabilités** | Chaque thread a un rôle clair |
| **Scalabilité** | Plusieurs clients/hostess/kiosk sans bottleneck |
| **Équité** | Alternance counter/kiosk évite famine |
| **Priorité** | Clients réservation traités en priorité |
| **Robustesse** | Alternatives en cas d'indisponibilité |
| **Trace** | Logs détaillés de toutes les opérations |
| **Synchronisation Correcte** | Mutex et condition variables évitent race conditions |

---

## 🔍 Conclusion

Ce projet démontre une compréhension complète de :
- ✅ **Concurrence** : Multiples threads interagissant sans deadlock
- ✅ **Synchronisation** : Mutex et condition variables pour data integrity
- ✅ **Conception** : Patterns de producteur-consommateur clairs
- ✅ **Algorithmes** : Alternance équitable, logique de priorité
- ✅ **Gestion d'Erreurs** : Alternatives intelligentes en cas de conflit

Le système est une simulation réaliste d'un cinéma moderne avec gestion complète des réservations, ventes, et synchronisation multi-thread.
