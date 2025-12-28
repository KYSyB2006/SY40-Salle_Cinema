PROJET GESTION CINEMA - MODULE INTERFACE & FLUX
===============================================
Auteur : Responsable 3
Version : 1.0 (Prod)
Date : 28/12/2024

DESCRIPTION
-----------
Application de gestion de flux cinéma en langage C (Console/TUI).
Ce module gère les interfaces clients (Guichet), Staff (Hôtesse), 
les files d'attente prioritaires et le reporting statistique.

FONCTIONNALITÉS CLÉS (Conforme Cahier des Charges)
--------------------------------------------------
1. Guichet Automatique :
   - Visualisation graphique de la salle (Plan A1-E10).
   - Achat atomique (Impossible de réserver deux fois la même place).
   - Gestion des priorités (Clients avec réservation passent devant).

2. Interface Hôtesse :
   - Vérification de l'âge pour les films interdits.
   - Gestion des échanges sans frais.

3. Administration & Alertes :
   - Calcul du Taux de Remplissage (%).
   - Système de Notification de Masse (Simulation SMS/Email).
   - Export automatique des logs dans 'cinema_stats.log'.

COMPILATION & INSTALLATION
--------------------------
Pré-requis : GCC (MinGW sous Windows)

Commande de compilation :
   gcc main_ui.c queue_manager.c reporting.c -o cinema_app -Wall

Exécution :
   .\cinema_app.exe (Windows)
   ./cinema_app (Linux/Mac)

STRUCTURE DES FICHIERS
----------------------
- main_ui.c       : Point d'entrée, Menus, Logique d'affichage (Carte).
- queue_manager.c : Algorithmes de tri (Listes chaînées), Alertes.
- reporting.c     : Calculs financiers, Stats, Ecriture fichier log.
- cinema_ui.h     : Déclarations globales, Structures de données.

TESTS RAPIDES
-------------
Utilisez le menu Admin (Option 3) -> Option 2 pour tester l'alerte de masse.
Utilisez le Guichet (Option 1) pour voir la carte des sièges interactive.