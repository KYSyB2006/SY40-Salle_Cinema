# Test du Capacity Monitor

## Description

Le fichier `test_capacity_monitor.c` contient un test complet du système de monitoring de capacité développé aujourd'hui. Ce test simule des achats progressifs de billets et vérifie que:

1. ✓ Le moniteur se lance correctement
2. ✓ L'alerte SIGUSR2 est envoyée quand une séance atteint 90% de capacité
3. ✓ Les achats de billets sont bloqués après le seuil de 90%

## Compilation

Le test a déjà été compilé automatiquement. Pour recompiler:

```bash
gcc -Wall -Wextra test_capacity_monitor.c gestion.c alternatives.c ticket_service.c \
    reservation_service.c threads.c client.c alarm.c -o test_capacity_monitor -lpthread
```

## Exécution

```bash
./test_capacity_monitor
```

## Déroulement du test

### Phase 1: Initialisation
- Création d'un cinéma avec une salle 3x3 (9 places)
- Ajout d'un film "Matrix Reloaded" (catégorie -16 ans)
- Création d'une séance pour ce film
- Initialisation du moniteur de capacité

### Phase 2: Achats progressifs
- Achat de 8 billets (88.9% de la capacité -> 8 billets sur 9 places)
- Un billet toutes les secondes pour voir l'alerte en temps réel
- **À 8 billets vendus (~89%), l'alerte SIGUSR2 doit être déclenchée**

### Phase 3: Tentative d'achat à capacité pleine
- Tentative d'achat d'un 9e billet
- Vérification que l'achat est refusé (code SEAT_UNAVAILABLE)

### Phase 4: Résultats et nettoyage
- Affichage des statistiques
- Arrêt du moniteur
- Libération des ressources

## Résultats attendus

```
PHASE 1 - INITIALISATION:
✓ Salle créée: 'Salle Premium' (9 places)
✓ Film créé: 'Matrix Reloaded' (Catégorie: -16 ans)
✓ Seance creee (ID: 0)
✓ Moniteur de capacité initialisé
✓ Moniteur demarré (intervalle: 500ms)

PHASE 2 - ACHATS:
✓ Billet 1 acheté pour la séance 0 (Siège 1)
✓ Billet 2 acheté pour la séance 0 (Siège 2)
...
✓ Billet 8 acheté pour la séance 0 (Siège 8)

╔═══════════════════════════════════╗
║ [ALERTE CAPACITE] Signal SIGUSR2  ║
║ Nombre d'alertes: 1               ║
╚═══════════════════════════════════╝

PHASE 3 - TENTATIVE D'ACHAT À CAPACITÉ PLEINE:
✗ Achat REFUSÉ (comportement attendu)

PHASE 4 - RÉSULTATS:
📊 Billets vendus: 8 / 9
✓ Taux d'occupation: 88.9%
✓ TEST RÉUSSI: L'alerte capacité a été correctement déclenchée!
```

## Signification des symboles

- ✓ : Succès
- ✗ : Erreur ou action bloquée
- 📊 : Statistiques
- 🚨 : Alerte/Signal

## Modification des paramètres

Vous pouvez modifier le test en changeant:

- **Taille de la salle** : Dans le `main()`, la salle est 3x3. Changez `room_create("Salle Premium", 3, 3);`
- **Nombre de billets achetés** : Modifiez `args->num_tickets = 8;`
- **Intervalle de vérification du moniteur** : Changez `capacity_monitor_start(capacity_monitor, 500);` (en millisecondes)
- **Seuil d'alerte** : Modifiez `FULL_CAPACITY_THRESHOLD` dans `alarm.h` (en %)

## Dépannage

### Le test ne compile pas
- Vérifiez que tous les fichiers source sont présents: `gestion.c`, `alternatives.c`, `ticket_service.c`, `reservation_service.c`, `threads.c`, `client.c`, `alarm.c`
- Vérifiez que les fichiers header correspondants existent

### Aucune alerte n'est affichée
- Vérifiez que le moniteur a bien démarré (message "✓ Moniteur demarré")
- Vérifiez que le handler SIGUSR2 a bien été enregistré (ligne `signal(SIGUSR2, handle_sigusr2);`)
- Augmentez le temps d'attente ou diminuez l'intervalle du moniteur

### Les achats ne sont pas bloqués à 90%
- Vérifiez que `check_full_capacity_and_alert()` est bien appelé dans `purchase_ticket()`
- Vérifiez la valeur de `FULL_CAPACITY_THRESHOLD` (devrait être 90.0)

## Fichiers impliqués

- `test_capacity_monitor.c` : Le test lui-même
- `alarm.h` / `alarm.c` : Implémentation du système de monitoring
- `ticket_service.c` : Intégration du blocage des achats
- `gestion.c` : Gestion du cinéma
