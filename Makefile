# Options de compilation
CFLAGS = -Wall -Wextra 

# Liste des exécutables et des fichiers associés
# Format : EXEC = fichier1.o fichier2.o ...
#EXEC1 = cinema_test
#EXEC1_SRCS = client.c test_gestion_cinema.c gestion.c 

EXEC2 = ticket_test
EXEC2_SRCS = client.c test_gestion_ticket.c gestion.c alternatives.c ticket_service.c reservation_service.c threads.c

# Liste de tous les exécutables
EXES = $(EXEC2) #$(EXEC1) 

# Règle par défaut : compiler tous les exécutables
all: $(EXES)

# Compilation des exécutables
#$(EXEC1): $(EXEC1_SRCS)
#	gcc $(CFLAGS) $(EXEC1_SRCS) -o $(EXEC1)

$(EXEC2): $(EXEC2_SRCS)
	gcc $(CFLAGS) $(EXEC2_SRCS) -o $(EXEC2)

# Nettoyer tous les exécutables
clean:
	rm -f $(EXES)
