# Options de compilation
CFLAGS = -Wall -Wextra 
LDFLAGS = -lpthread

# Liste des exécutables et des fichiers associés
# Format : EXEC = fichier1.o fichier2.o ...
#EXEC1 = cinema_test
#EXEC1_SRCS = client.c test_gestion_cinema.c gestion.c 

EXEC2 = ticket_test
EXEC2_SRCS = client.c test_gestion_ticket.c gestion.c alternatives.c ticket_service.c reservation_service.c threads.c alarm.c

EXEC3 = test_capacity_monitor
EXEC3_SRCS = test_capacity_monitor.c gestion.c alternatives.c ticket_service.c reservation_service.c threads.c client.c alarm.c

# Liste de tous les exécutables
EXES = $(EXEC2) $(EXEC3) #$(EXEC1) 

# Règle par défaut : compiler tous les exécutables
all: $(EXES)

# Compilation des exécutables
#$(EXEC1): $(EXEC1_SRCS)
#	gcc $(CFLAGS) $(EXEC1_SRCS) -o $(EXEC1)

$(EXEC2): $(EXEC2_SRCS)
	gcc $(CFLAGS) $(EXEC2_SRCS) -o $(EXEC2) $(LDFLAGS)

$(EXEC3): $(EXEC3_SRCS)
	gcc $(CFLAGS) $(EXEC3_SRCS) -o $(EXEC3) $(LDFLAGS)
clean:
	rm -f $(EXES)
