# Nom des exécutables
CLIENT = client
SERVER = server

# Dossiers
CLIENT_DIR = ClientSrc
SERVER_DIR = ServeurSrc

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP

# Fichiers sources
CLIENT_SRC = $(CLIENT_DIR)/client.c
SERVER_SRC = $(SERVER_DIR)/server.c \
             $(SERVER_DIR)/awale.c \
             $(SERVER_DIR)/historique.c \
             $(SERVER_DIR)/amis.c \
             $(SERVER_DIR)/io.c

# Objets
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Fichiers de dépendances générés automatiquement (.d)
CLIENT_DEP = $(CLIENT_OBJ:.o=.d)
SERVER_DEP = $(SERVER_OBJ:.o=.d)

# Règle par défaut
all: $(CLIENT) $(SERVER)

# Compilation du client
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation du serveur
$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Règles de compilation pour chaque dossier
$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_DEP) $(SERVER_DEP)

fclean: clean
	rm -f $(CLIENT) $(SERVER)

re: fclean all

.PHONY: all clean fclean re

# Inclusion silencieuse des fichiers de dépendances
-include $(CLIENT_DEP) $(SERVER_DEP)
