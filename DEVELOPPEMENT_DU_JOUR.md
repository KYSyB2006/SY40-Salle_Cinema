# Résumé des développements du jour

## Vue d'ensemble

Aujourd'hui, un système complet de surveillance de capacité a été implémenté. Quand une séance atteint 90% de remplissage, le système envoie un signal SIGUSR2 et bloque la vente de nouveaux billets.

---

## 1. Fonctions développées

### 1.1 check_full_capacity_and_alert()
**Fichier**: `alarm.c` (lignes 179-211)

Fonction qui:
- Vérifie si une séance a atteint 90% de capacité
- Envoie un signal SIGUSR2 si le seuil est atteint
- Affiche une alerte avec les détails de la séance et de la salle
- Retourne 1 si l'alerte est envoyée, 0 si le seuil n'est pas atteint, -1 en erreur

```c
int check_full_capacity_and_alert(Cinema* cinema, int screening_id)
```

### 1.2 Système de monitoring avec thread (CapacityMonitor)
**Fichiers**: `alarm.h` (lignes 27-33), `alarm.c` (lignes 279-360)

#### Structure CapacityMonitor
```c
typedef struct {
    Cinema* cinema;
    pthread_t thread;
    pthread_mutex_t mutex;
    int active;
    int interval_ms;
    int alerted_screenings[MAX_SCREENINGS];  // Tracker des séances
} CapacityMonitor;
```

#### Fonctions de gestion
- `capacity_monitor_init(Cinema* cinema)` - Initialise le moniteur
- `capacity_monitor_start(CapacityMonitor* monitor, int interval_ms)` - Démarre le thread
- `capacity_monitor_stop(CapacityMonitor* monitor)` - Arrête le thread
- `capacity_monitor_destroy(CapacityMonitor* monitor)` - Libère les ressources

#### Fonctions internes
- `check_and_alert_all_screenings()` - Vérifie toutes les séances
- `capacity_monitor_thread()` - Fonction du thread (boucle infinie)

**Caractéristiques**:
- Thread-safe avec mutex
- Intervalle de vérification configurable
- Chaque séance n'est alertée qu'une fois quand le seuil est atteint
- Flag se réinitialise si l'occupation redescend sous 90%

---

## 2. Modifications aux fichiers existants

### 2.1 ticket_service.c
**Lignes modifiées**: 17-18, 65-80

- Ajout de `#include "alarm.h"`
- Intégration dans `purchase_ticket()`:
  ```c
  // Vérification de la capacité (90%)
  check_full_capacity_and_alert(cinema, screening_id);
  if (cinema->statistics->occupancy_rate_by_screening[screening_id] >= 90.0f) {
      // Refuser l'achat
      return SEAT_UNAVAILABLE;
  }
  ```

### 2.2 alarm.h
**Lignes modifiées**: 9, 27-39

- Ajout de `#define FULL_CAPACITY_THRESHOLD 90.0f`
- Ajout de la structure `CapacityMonitor`
- Ajout des 4 fonctions de gestion du moniteur
- Ajout de la fonction `check_full_capacity_and_alert()`

### 2.3 alarm.c
**Lignes modifiées**: 179-360

- Implémentation de `check_full_capacity_and_alert()`
- Implémentation du système de monitoring (119 lignes de code)
- Ajout de la fonction thread `capacity_monitor_thread()`
- Ajout de la fonction interne `check_and_alert_all_screenings()`

### 2.4 test_gestion_ticket.c
**Lignes modifiées**: 8, 19-22, 51-54

- Ajout de `#include <signal.h>` et `#include "alarm.h"`
- Ajout du handler `handle_sigusr2()`
- Initialisation et démarrage du moniteur dans `main()`

---

## 3. Fichiers créés

### 3.1 test_capacity_monitor.c
**Type**: Fichier de test complet et autonome

Contient un programme de test qui:
1. Initialise un cinéma avec une salle 3x3 et une séance
2. Initialise et démarre le CapacityMonitor
3. Simule des achats progressifs de billets
4. Vérifie que SIGUSR2 est envoyé à 90%
5. Vérifie que les achats sont bloqués à 90%+
6. Affiche les statistiques

**Exécution**: `./test_capacity_monitor`

### 3.2 CAPACITY_MONITOR_README.md
**Type**: Documentation complète du système

Contient:
- Description du CapacityMonitor
- Fonctionnalités principales
- Guide d'utilisation
- Exemple complet de code
- Documentation API détaillée
- Notes d'implémentation

### 3.3 TEST_CAPACITY_MONITOR_README.md
**Type**: Guide d'exécution du test

Contient:
- Description du test
- Instructions de compilation et exécution
- Déroulement détaillé des phases
- Résultats attendus
- Conseils de dépannage

---

## 4. Signal SIGUSR2

Le signal **SIGUSR2** est envoyé chaque fois qu'une séance atteint 90% de capacité.

### Enregistrement du handler

```c
signal(SIGUSR2, handle_sigusr2);
```

### Exemple de handler

```c
void handle_sigusr2(int sig) {
    printf("[ALERTE] Une séance a atteint 90%% de capacité!\n");
}
```

---

## 5. Intégration système

### Flux complet

1. **Achat d'un billet** → `purchase_ticket()` appelée
2. **Vérification de capacité** → `check_full_capacity_and_alert()` vérifiée
3. **Si >= 90%** → Signal SIGUSR2 envoyé
4. **Aussi en parallèle** → CapacityMonitor thread vérifie continuellement
5. **Achat bloqué** → `SEAT_UNAVAILABLE` retourné si >= 90%

### Avantages

- ✓ Double vérification (transaction + monitoring continu)
- ✓ Non-bloquant (thread indépendant)
- ✓ Thread-safe (mutex)
- ✓ Configurable (intervalle ajustable)
- ✓ Intelligent (une alerte par séance)

---

## 6. Constantes et seuils

### alarm.h

```c
#define MAX_SCREENINGS 200
#define OCCUPANCY_THRESHOLD 20.0f    // Pour can_change
#define FULL_CAPACITY_THRESHOLD 90.0f  // Pour l'alerte 90%
```

---

## 7. Tests et validation

### Compilation
```bash
gcc -Wall -Wextra test_capacity_monitor.c gestion.c alternatives.c \
    ticket_service.c reservation_service.c threads.c client.c alarm.c \
    -o test_capacity_monitor -lpthread
```

### Exécution
```bash
./test_capacity_monitor
```

### Résultats esperés
- ✓ 8 billets vendus (89%)
- ✓ 1 signal SIGUSR2 reçu
- ✓ Tentative 9e billet refusée
- ✓ Messages d'alerte affichés

---

## 8. Statistiques du code

| Fichier | Lignes | Type |
|---------|--------|------|
| alarm.h | +12 (typedef + déclarations) | Headers |
| alarm.c | +180 (fonctions) | Implementation |
| ticket_service.c | +15 (vérification capacité) | Integration |
| test_gestion_ticket.c | +4 (initialization) | Test |
| test_capacity_monitor.c | 215 | Test autonome |

**Total ajouté**: ~426 lignes de code

---

## 9. Points clés à retenir

1. **CapacityMonitor** = thread + mutex + occupancy tracking
2. **SIGUSR2** = signal envoyé automatiquement à 90%
3. **Blocage des ventes** = intégré dans `purchase_ticket()`
4. **Double protection** = vérification à la transaction + monitoring continu
5. **Intelligent** = une alerte par séance (pas de spam)

---

## 10. Prochaines étapes possibles

- Ajouter un handler SIGUSR2 personnalisé
- Notifier les clients de la capacité pleine
- Logger les alertes dans un fichier
- Créer une queue de waiting list
- Implémenter une gestion des remboursements
- Ajouter des statistiques par heure

---

## Conclusion

Le système de surveillance de capacité à 90% est maintenant **complètement opérationnel** et **testable** via le fichier `test_capacity_monitor.c`.

Le système combine:
- ✓ Vérification immédiate lors de chaque achat
- ✓ Monitoring continu en arrière-plan
- ✓ Signal système (SIGUSR2) pour les alertes
- ✓ Blocage automatique des ventes
- ✓ Thread-safety avec mutex
- ✓ Gestion intelligente des alertes
