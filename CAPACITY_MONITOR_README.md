# Capacity Monitor - Surveillance de la Capacité à 90%

## Description

Le `CapacityMonitor` est un système de surveillance qui fonctionne en arrière-plan avec un thread dédié. Il vérifie continuellement le taux d'occupation de toutes les séances du cinéma et déclenche automatiquement une alerte SIGUSR2 lorsqu'une séance atteint 90% de capacité.

## Fonctionnalités

- **Surveillance continue** : Le thread s'exécute périodiquement (intervalle configurable, par défaut 1 seconde)
- **Signal SIGUSR2** : Envoi d'un signal d'alerte au processus principal
- **Blocage des ventes** : Les billets ne peuvent plus être vendus pour une séance à 90%+
- **Alerte simple** : Chaque séance n'est alertée qu'une seule fois quand le seuil est atteint
- **Réinitialisation** : Le flag d'alerte se réinitialise si l'occupation redescend sous 90%

## Utilisation

### 1. Initialisation

```c
#include "alarm.h"

// Créer et initialiser le moniteur
CapacityMonitor* monitor = capacity_monitor_init(cinema);
if (!monitor) {
    fprintf(stderr, "Erreur lors de l'initialisation du moniteur\n");
    return -1;
}
```

### 2. Démarrage du monitoring

```c
// Démarrer le thread de surveillance avec un intervalle de 1000 ms
int result = capacity_monitor_start(monitor, 1000);  // 1 seconde
if (result != 0) {
    fprintf(stderr, "Erreur au démarrage du moniteur\n");
    capacity_monitor_destroy(monitor);
    return -1;
}
```

### 3. Arrêt et nettoyage

```c
// Arrêter le monitoring
capacity_monitor_stop(monitor);

// Libérer les ressources
capacity_monitor_destroy(monitor);
```

## Exemple complet

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "gestion.h"
#include "alarm.h"

// Handler pour SIGUSR2
void handle_sigusr2(int sig) {
    printf("[SIGNAL REÇU] SIGUSR2 - Une séance a atteint 90% de capacité!\n");
}

int main() {
    signal(SIGUSR2, handle_sigusr2);
    
    // Initialiser le cinéma
    Cinema* cinema = cinema_create(5);
    
    // Ajouter des films, salles et séances...
    // ...
    
    // Initialiser et démarrer le moniteur de capacité
    CapacityMonitor* capacity_monitor = capacity_monitor_init(cinema);
    capacity_monitor_start(capacity_monitor, 1000);  // Vérification chaque seconde
    
    // ... Laisser fonctionner le cinéma ...
    sleep(30);
    
    // Arrêter le monitoring
    capacity_monitor_stop(capacity_monitor);
    capacity_monitor_destroy(capacity_monitor);
    
    return 0;
}
```

## Fonctions disponibles

### `CapacityMonitor* capacity_monitor_init(Cinema* cinema)`
**Initialise** un nouveau moniteur de capacité.
- **Paramètre** : `cinema` - pointeur vers la structure Cinema
- **Retour** : Pointeur vers CapacityMonitor (NULL en cas d'erreur)

### `int capacity_monitor_start(CapacityMonitor* monitor, int interval_ms)`
**Démarre** le thread de surveillance.
- **Paramètres** :
  - `monitor` - pointeur vers CapacityMonitor
  - `interval_ms` - intervalle de vérification en millisecondes
- **Retour** : 0 en cas de succès, -1 en cas d'erreur

### `void capacity_monitor_stop(CapacityMonitor* monitor)`
**Arrête** le thread de surveillance.
- **Paramètre** : `monitor` - pointeur vers CapacityMonitor

### `void capacity_monitor_destroy(CapacityMonitor* monitor)`
**Libère** les ressources du moniteur.
- **Paramètre** : `monitor` - pointeur vers CapacityMonitor

## Détails d'implémentation

### Signal SIGUSR2
Lorsqu'une séance atteint 90% de capacité, le moniteur envoie un signal SIGUSR2 au processus principal. Vous pouvez installer un handler pour réagir à ce signal.

### Seuil de remplissage
Le seuil de 90% est défini par la constante `FULL_CAPACITY_THRESHOLD` dans `alarm.h`.

### Avantages du système
1. **Thread-safe** : Utilise un mutex pour sécuriser l'accès aux données
2. **Non-bloquant** : N'interfère pas avec le traitement normal des billets
3. **Intelligent** : Ne notifie qu'une seule fois par séance
4. **Configurable** : L'intervalle de vérification peut être ajusté

## Intégration avec purchase_ticket

La fonction `purchase_ticket` dans `ticket_service.c` intègre automatiquement la vérification de capacité. Quand une séance atteint 90%, les achats de billets sont refusés pour cette séance.

## Notes importantes

- Le moniteur doit être arrêté avant de détruire la structure Cinema
- Le signal SIGUSR2 est envoyé de manière asynchrone
- Chaque séance ne déclenche l'alerte qu'une seule fois quand le seuil est atteint
