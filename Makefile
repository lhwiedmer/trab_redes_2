# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -g

# Binários TCP
BIN_CLIENT = client/client
BIN_SERVER = server/server
BIN_MIRROR = mirror/mirror

# Arquivos-fonte TCP
SRC_CLIENT = client/client.c
SRC_SERVER = server/server.c
SRC_MIRROR = mirror/mirror.c


# Alvo padrão: compila cliente e servidor TCP
all: $(BIN_CLIENT) $(BIN_SERVER) $(BIN_MIRROR)

# Compila cliente TCP
$(BIN_CLIENT): $(SRC_CLIENT) message.o utils.o list.o
	$(CC) $(CFLAGS) -o $@ $^

# Compila servidor TCP
$(BIN_SERVER): $(SRC_SERVER) message.o utils.o list.o
	$(CC) $(CFLAGS) -o $@ $^ 

# Compila mirror TCP
$(BIN_MIRROR): $(SRC_MIRROR) message.o utils.o list.o
	$(CC) $(CFLAGS) -o $@ $^ 

message.o: message.c message.h
	$(CC) $(CFLAGS) -c message.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

# Limpeza dos binários TCP
clean:
	rm -f $(BIN_CLIENT) $(BIN_SERVER) $(BIN_MIRROR) *.o