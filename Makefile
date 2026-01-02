# Options de compilation
CFLAGS = -Wall -Wextra -Werror

# Liste des exécutables et des fichiers associés
# Format : EXEC = fichier1.o fichier2.o ...
EXEC1 = cinema_test
EXEC1_SRCS = test_gestion_cinema.c gestion.c

#EXEC2 = test2
#EXEC2_SRCS = test2.c utils.c

# Liste de tous les exécutables
EXES = $(EXEC1)

# Règle par défaut : compiler tous les exécutables
all: $(EXES)

# Compilation des exécutables
$(EXEC1): $(EXEC1_SRCS)
	gcc $(CFLAGS) $(EXEC1_SRCS) -o $(EXEC1)


# Nettoyer tous les exécutables
clean:
	rm -f $(EXES)
