# Nom des exécutables
CLIENT = client
SERVER = server

# Dossiers
CLIENT_DIR = ClientSrc
SERVER_DIR = ServeurSrc

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra

# Fichiers sources et objets
CLIENT_SRC = $(CLIENT_DIR)/client.c
SERVER_SRC = $(SERVER_DIR)/server.c $(SERVER_DIR)/awale.c $(SERVER_DIR)/historique.c $(SERVER_DIR)/amis.c $(SERVER_DIR)/io.c

CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Règle par défaut
all: $(CLIENT) $(SERVER)

# Compilation du client
$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation du serveur
$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation des fichiers .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers objets et exécutables
clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ)

fclean: clean
	rm -f $(CLIENT) $(SERVER)

# Recompilation complète
re: fclean all

# Indique à make que ces noms ne sont pas des fichiers
.PHONY: all clean fclean re
