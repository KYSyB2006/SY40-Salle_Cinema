/* Dans ce fichier, on va définir les fonctions pour la création, la modification,
   l'annulation des réservations
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include "struct.h"


// Création d'une réservation
// Modification d'une réservation
// Validation d'une réservation
// Annulation d'une réservation