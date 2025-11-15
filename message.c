/**
 * @name message.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Fonte com funções tratando cada comando
 */

#include "message.h"

#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/stat.h>    // Informações de arquivos
#include <sys/types.h>   // Tipos de dados
#include <unistd.h>      // close, read, write

void sendMessage(int sockfd, unsigned char* buffer, unsigned long n) {
    if (send(sockfd, buffer, n, 0) == -1) {
        perror("Falha ao tentar enviar mensagem\n");
        exit(1);
    }
}

int rcvMessage(int sockfd, unsigned char* buffer, unsigned long n) {
    if (recv(sockfd, buffer, n, 0) == -1) {
        perror("Falha ao tentar receber mensagem\n");
        exit(1);
    }
    if (buffer[0] == ERROR) {
        return -1;
    }
    return 1;
}

/**
 * @brief Envia os metadados de um arquivo
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] fileName Nome do arquivo
 * @param[in] size Tamanho do arquivo
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int sendFileInfo(int sockfd, const char* fileName, unsigned long size) {
    unsigned char buffer[TAM_MAX];

    // Monta mensagem com metadados do arquivo
    buffer[0] = FILEINFO;
    memcpy(buffer + 1, &size, 8);              // Tamanho do arquivo
    buffer[9] = strlen(fileName);              // Tamanho do nome do arquivo
    memcpy(buffer + 10, fileName, buffer[9]);  // Nome do arquivo

    sendMessage(sockfd, buffer, buffer[9] + 10);
    return rcvMessage(sockfd, buffer, TAM_MAX);
}

int sendFile(int sockfd, const char* fileName) {
    struct stat st;
    if (stat(fileName, &st) != 0) {
        perror("Falha ao tentar encontrar arquivo\n");
        exit(1);
    }
    unsigned long size = st.st_size;
    const char* base = strrchr(fileName, '/');  // Pega apenas o nome do arquivo
    if (!base) {
        base = fileName;
    } else {
        base++;
    }

    if (sendFileInfo(sockfd, base, size) == -1) {
        return -1;
    }
    FILE* arq = fopen(fileName, "r");
    if (!arq) {
        perror("Falha ao tentar abrir arquivo\n");
        exit(1);
    }

    int reps = size / (TAM_MAX - 1);
    reps++;

    unsigned char buffer[TAM_MAX];
    buffer[0] = DATA;

    printf("Enviando arquivo '%s'...\n", fileName);

    for (int i = 0; i < reps; i++) {
        fread(buffer + 1, 1, TAM_MAX - 1, arq);
        sendMessage(sockfd, buffer, TAM_MAX);
    }

    fclose(arq);

    if (rcvMessage(sockfd, buffer, TAM_MAX) == 1) {
        if (buffer[1] == 0) {
            printf(
                "Envio concluído. Arquivo replicado apenas para o servidor "
                "principal\n");
        } else {
            printf(
                "Envio concluído. Arquivo replicado com sucesso para %d "
                "servidores "
                "réplica.\n",
                buffer[1]);
        }
        return 1;
    }
    return -1;
}

int rcvFile(int sockfd, unsigned char* buffer) {
    unsigned long size;
    memcpy(&size, buffer + 1, 8);
    unsigned char nameSize = buffer[9];
    char name[nameSize + 1];
    for (int i = 0; i < nameSize; i++) {
        name[i] = buffer[i + 10];
    }
    name[nameSize] = '\0';

    printf("Tamanho: %ld\nNome: %s\n", size, name);

    struct stat st;
    if (stat(FILE_DIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            // cria
            if (mkdir(FILE_DIR, 0755) != 0) {
                // mkdir falhou
                return -1;
            }
        }
    } else {
        // cria
        if (mkdir(FILE_DIR, 0755) != 0) {
            // mkdir falhou
            return -1;
        }
    }

    char newName[sizeof(FILE_DIR) + nameSize + 2];  // files/<file>\0
    strcpy(newName, FILE_DIR);
    strcat(newName, name);

    FILE* arq = fopen(newName, "w");
    if (!arq) {
        perror("Falha ao tentar abrir arquivo\n");
        // Mandar mensagem?
    }
    buffer[0] = OK;
    sendMessage(sockfd, buffer, 1);

    int reps = size / (TAM_MAX - 1);
    int lastSize = size % (TAM_MAX - 1);

    for (int i = 0; i < reps; i++) {
        rcvMessage(sockfd, buffer, TAM_MAX);
        fwrite(buffer + 1, 1, TAM_MAX - 1, arq);
    }

    rcvMessage(sockfd, buffer, TAM_MAX);
    fwrite(buffer + 1, 1, lastSize, arq);
    fclose(arq);

    buffer[0] = OK;
    buffer[1] = 0;
    sendMessage(sockfd, buffer, 1);

    return 1;
}

int reqList(int sockdfd) { return 1; }