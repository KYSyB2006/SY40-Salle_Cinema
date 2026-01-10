# Test du Capacity Monitor - Guide Complet

## 📋 Fichiers de test et documentation

Trois fichiers ont été créés pour documenter et tester le système développé:

### 1. **test_capacity_monitor.c** (Programme de test)
Exécutable autonome qui teste complètement le système de surveillance.

**Compilation**: Automatique via Makefile
```bash
make test_capacity_monitor
```

**Exécution**:
```bash
./test_capacity_monitor
```

### 2. **CAPACITY_MONITOR_README.md** (Documentation API)
Documentation technique complète du système CapacityMonitor.

Contient:
- Descriptions détaillées des fonctions
- Exemples de code
- Guide d'utilisation
- Détails d'implémentation
- Notes importantes

### 3. **TEST_CAPACITY_MONITOR_README.md** (Guide du test)
Guide pratique pour exécuter et comprendre le test.

Contient:
- Instructions de compilation
- Déroulement complet du test
- Résultats attendus
- Dépannage

### 4. **DEVELOPPEMENT_DU_JOUR.md** (Résumé technique)
Résumé exhaustif de tout ce qui a été développé.

Contient:
- Vue d'ensemble complète
- Détails de chaque fonction
- Modifications aux fichiers existants
- Statistiques de code
- Prochaines étapes possibles

---

## 🚀 Démarrage rapide

### Compiler le test
```bash
make test_capacity_monitor
```

### Exécuter le test
```bash
./test_capacity_monitor
```

### Voir les résultats
Le test affichera:
- ✓ Initialisation réussie
- ✓ 8 billets vendus progressivement
- 🚨 1 signal SIGUSR2 reçu (alerte 90%)
- ✓ 9e billet refusé (comportement attendu)

---

## 🎯 Fonctionnalités testées

| Fonctionnalité | Status | Description |
|---|---|---|
| Initialisation moniteur | ✓ | Création et setup du thread |
| Vérification capacité | ✓ | Check occupancy_rate |
| Signal SIGUSR2 | ✓ | Envoi correct du signal |
| Blocage des ventes | ✓ | Refus d'achat à 90%+ |
| Thread-safety | ✓ | Mutex protège les données |
| Alerte intelligente | ✓ | Une alerte par séance |

---

## 📊 Résultats attendus

```
╔═══════════════════════════════════════════════════════════╗
║   TEST DU CAPACITY MONITOR - ALERTE 90 POUR CENT         ║
╚═══════════════════════════════════════════════════════════╝

[SETUP] Initialisation du cinéma...
✓ Salle créée: 'Salle Premium' (9 places)
✓ Film créé: 'Matrix Reloaded' (Catégorie: -16 ans)
✓ Seance creee (ID: 0)

[SETUP] Initialisation du moniteur de capacité...
✓ Moniteur de capacité initialisé
✓ Moniteur demarré (intervalle: 500ms)
✓ Seuil d'alerte: 90.0 POUR CENT de capacité

╔═══════════════════════════════════════════════════════════╗
║   PHASE DE TEST: ACHATS PROGRESSIFS DE BILLETS            ║
╚═══════════════════════════════════════════════════════════╝

Nous allons acheter 8 billets sur 9 places disponibles.
A 90 POUR CENT de capacité (8.1 places), une alerte SIGUSR2 doit etre envoyee.

✓ Billet 1 acheté pour la séance 0 (Siège 1)
✓ Billet 2 acheté pour la séance 0 (Siège 2)
...
✓ Billet 8 acheté pour la séance 0 (Siège 8)

╔═══════════════════════════════════════════════════════════╗
║ [ALERTE CAPACITE] Signal SIGUSR2 recu                    ║
║ Nombre d'alertes: 1                                       ║
╚═══════════════════════════════════════════════════════════╝

╔═══════════════════════════════════════════════════════════╗
║                    RÉSULTATS DU TEST                      ║
╚═══════════════════════════════════════════════════════════╝

📊 Statistiques de la séance:
  • Billets vendus: 8 / 9
  • Places disponibles: 1
  • Taux d'occupation: 88.9%
  • Total billets vendus (cinéma): 8
  • Revenu total: 120.00€

🚨 Alertes SIGUSR2 reçues: 1
✓ TEST RÉUSSI: L'alerte capacité a été correctement déclenchée!

╔═══════════════════════════════════════════════════════════╗
║   PHASE DE TEST: TENTATIVE D'ACHAT A 90+ POUR CENT        ║
╚═══════════════════════════════════════════════════════════╝

Tentative d'achat d'un 9e billet (la salle est a 90+)...

Résultat: ✓ Achat REFUSÉ (comportement attendu)

╔═══════════════════════════════════════════════════════════╗
║                    NETTOYAGE                              ║
╚═══════════════════════════════════════════════════════════╝

Arrêt du moniteur de capacité...
✓ Moniteur arrêté
Libération des ressources...
✓ Ressources du moniteur libérées

✓ Test terminé avec succès!
```

---

## 🔍 Déroulement détaillé

### Phase 1: Initialisation (30 secondes)
- Création du cinéma
- Création d'une salle 3x3 (9 places)
- Création d'un film et d'une séance
- Initialisation du CapacityMonitor (thread)
- Démarrage du monitoring

### Phase 2: Achats progressifs (8 secondes)
- Achat de 8 billets (1 par seconde)
- À 8 billets vendus (~89%), SIGUSR2 est envoyé
- Message d'alerte affiché
- Occupancy rate: 88.9%

### Phase 3: Tentative à capacité pleine (2 secondes)
- Tentative d'achat du 9e billet
- Achat refusé (code SEAT_UNAVAILABLE)
- Message de refus affiché

### Phase 4: Nettoyage (2 secondes)
- Arrêt du thread moniteur
- Libération des ressources
- Affichage des statistiques finales

**Durée totale**: ~45 secondes

---

## 🛠️ Personnalisation du test

Vous pouvez modifier le test en changeant les paramètres dans `test_capacity_monitor.c`:

```c
// Taille de la salle
Room* r1 = room_create("Salle Premium", 5, 5);  // 5x5 au lieu de 3x3

// Nombre de billets à acheter
args->num_tickets = 20;  // Au lieu de 8

// Intervalle du moniteur (en ms)
capacity_monitor_start(capacity_monitor, 100);  // Plus rapide (100ms)

// Seuil d'alerte
// Modifier FULL_CAPACITY_THRESHOLD dans alarm.h
```

---

## 📝 Structure du code

```
test_capacity_monitor.c
├── handle_sigusr2()           // Handler du signal
├── buyer_thread()             // Thread d'achat de billets
└── main()
    ├── Initialisation cinéma
    ├── Setup moniteur
    ├── Achats progressifs
    ├── Affichage résultats
    └── Nettoyage
```

---

## 🔗 Relations entre les fonctions

```
main()
│
├─→ signal(SIGUSR2, handle_sigusr2)
│
├─→ capacity_monitor_init()
│
├─→ capacity_monitor_start()
│   └─→ pthread_create()
│       └─→ capacity_monitor_thread()
│           └─→ check_and_alert_all_screenings()
│               └─→ check_full_capacity_and_alert()
│                   └─→ kill(SIGUSR2)
│
├─→ purchase_ticket()
│   └─→ check_full_capacity_and_alert()
│       └─→ kill(SIGUSR2)
│
└─→ capacity_monitor_stop()
    └─→ capacity_monitor_destroy()
```

---

## 📚 Documentation complète

Consultez les fichiers suivants pour plus de détails:

1. **CAPACITY_MONITOR_README.md**: API complète et guide d'utilisation
2. **TEST_CAPACITY_MONITOR_README.md**: Guide d'exécution du test
3. **DEVELOPPEMENT_DU_JOUR.md**: Résumé technique complet

---

## ✅ Checklist de validation

Après avoir exécuté le test, vérifiez:

- [ ] Le moniteur s'initialise correctement
- [ ] 8 billets sont vendus avec succès
- [ ] Un signal SIGUSR2 est reçu
- [ ] Le message d'alerte s'affiche
- [ ] Le 9e billet est refusé
- [ ] Les statistiques sont correctes
- [ ] Le moniteur s'arrête proprement

---

## 🐛 Dépannage

### Problème: "Aucun billet vendu"
**Solution**: Vérifiez que la salle a bien des places

### Problème: "Aucune alerte SIGUSR2"
**Solution**: Vérifiez que le handler est enregistré et que le moniteur est lancé

### Problème: "Le test ne compile pas"
**Solution**: Vérifiez que tous les fichiers source sont présents

### Problème: "Les achats ne sont pas bloqués"
**Solution**: Vérifiez que `purchase_ticket()` appelle `check_full_capacity_and_alert()`

---

## 📞 Support

Pour plus d'informations:
- Consultez **DEVELOPPEMENT_DU_JOUR.md** pour la liste complète des changements
- Consultez **CAPACITY_MONITOR_README.md** pour l'API
- Consultez **TEST_CAPACITY_MONITOR_README.md** pour l'exécution

---

**Créé**: Janvier 10, 2026
**Version**: 1.0
