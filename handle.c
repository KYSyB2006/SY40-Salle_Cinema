#include "handle.h"
#include <stdio.h>

// Handler pour SIGUSR2 (Alerte capacité 90%)
void handle_sigusr2(int sig) {
    (void)sig;  // Éviter le warning unused parameter
    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║ [ALERTE CAPACITE] Signal SIGUSR2 recu   ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
}

// Handler pour SIGUSR1 (Changement de film)
void handle_sigusr1(int sig) {
    (void)sig;
    printf("\n╔═══════════════════════════════════════════╗\n");
    printf("║ [ALERTE FILM] Signal SIGUSR1 recu       ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
}
