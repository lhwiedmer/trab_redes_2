/**
 * @name message.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Fonte com funções tratando cada comando
 */

#ifndef MESSAGE_C
#define MESSAGE_C

#include "message.h"

#include <dirent.h>
#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/stat.h>    // Informações de arquivos
#include <sys/types.h>   // Tipos de dados
#include <unistd.h>      // close, read, write

#include "list.h"
#include "utils.h"

void sendMessage(int sockfd, unsigned char* buffer, unsigned long n) {
    if (send(sockfd, buffer, n, 0) == -1) {
        perror("Falha ao tentar enviar mensagem\n");
        exit(1);
    }
}

int rcvMessage(int sockfd, unsigned char* buffer, unsigned long n) {
    memset(buffer, 0, n);
    if (recv(sockfd, buffer, n, 0) == -1) {
        perror("Falha ao tentar receber mensagem\n");
        exit(1);
    }
    if (buffer[0] == ERROR) {
        return -1;
    }
    return 1;
}

void endAll(int sockfd) {
    unsigned char a = END;
    sendMessage(sockfd, &a, 1);
    close(sockfd);
    return;
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

int sendFile(int sockfd, const char* fileName, int mode) {
    struct stat st;
    if (stat(fileName, &st) != 0) {
        return -2;
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

    if (mode == CLIENT_SERVER) {
        printf("Enviando arquivo '%s'...\n", fileName);
    }

    for (int i = 0; i < reps; i++) {
        fread(buffer + 1, 1, TAM_MAX - 1, arq);
        sendMessage(sockfd, buffer, TAM_MAX);
        if ((mode == CLIENT_SERVER) || (i + 1 < reps)) {
            if (rcvMessage(sockfd, buffer, TAM_MAX) == -1) {
                return -1;
            }
        }
    }

    fclose(arq);
    if (mode == CLIENT_SERVER) {
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
    }
    return 1;
}

int rcvFile(int sockfd, unsigned char* buffer, const char* dirPath) {
    unsigned long size;
    memcpy(&size, buffer + 1, 8);
    unsigned char nameSize = buffer[9];
    char name[nameSize + 1];
    for (int i = 0; i < nameSize; i++) {
        name[i] = buffer[i + 10];
    }
    name[nameSize] = '\0';

    if (createDir(dirPath) == -1) {
        return -1;
    }

    char newName[sizeof(dirPath) + nameSize + 2];  // files/<file>\0
    strcpy(newName, dirPath);
    strcat(newName, name);

    FILE* arq = fopen(newName, "w");
    if (!arq) {
        perror("Falha ao tentar abrir arquivo\n");
        return -1;
    }
    buffer[0] = OK;
    sendMessage(sockfd, buffer, 1);

    int reps = size / (TAM_MAX - 1);
    int lastSize = size % (TAM_MAX - 1);

    for (int i = 0; i < reps; i++) {
        rcvMessage(sockfd, buffer, TAM_MAX);
        fwrite(buffer + 1, 1, TAM_MAX - 1, arq);
        unsigned char a = OK;
        sendMessage(sockfd, &a, 1);
    }

    rcvMessage(sockfd, buffer, TAM_MAX);
    fwrite(buffer + 1, 1, lastSize, arq);
    fclose(arq);

    printf("Arquivo %s recebido. Armazenamento local concluído.\n", name);

    memset(buffer, 0, TAM_MAX);
    strcpy((char*)buffer, newName);

    return 1;
}

int reqList(int sockfd) {
    unsigned char a = LISTREQ;
    sendMessage(sockfd, &a, 1);

    char buffer[TAM_MAX];
    if (rcvMessage(sockfd, (unsigned char*)buffer, TAM_MAX) == -1) {
        return -1;
    }
    if (buffer[1] == NONE) {
        printf("O servidor não tem nenhum arquivo no diretório do usuário.\n");
        return 1;
    }
    while (buffer[1] == MORE) {
        printf("%s", buffer + 2);
        if (rcvMessage(sockfd, (unsigned char*)buffer, TAM_MAX) == -1) {
            printf("\n");
            return -1;
        }
    }
    printf("%s\n", buffer + 2);
    return 1;
}

int sendList(int sockfd, const char* dirPath) {
    unsigned char buffer[TAM_MAX];
    buffer[0] = LISTASW;

    DIR* dir;
    dir = opendir(dirPath);
    if (!dir) {
        buffer[1] = NONE;
        sendMessage(sockfd, buffer, 2);
        return 1;
    }

    printf("Recuperando listagem dos arquivos locais.\n");

    node_t* head = createNameList(dir, dirPath);
    if (!head) {
        buffer[1] = NONE;
        sendMessage(sockfd, buffer, 2);
        return 1;
    }

    printf("Enviando informações ao cliente.\n");

    int i = 2;

    node_t* aux = head;
    while (aux) {
        if (i + aux->size + 1 < TAM_MAX) {
            memcpy(buffer + i, aux->content, aux->size);
            i += aux->size;
            buffer[i++] = ' ';
            aux = aux->prox;
        } else {
            // manda o buffer e reinicia
            buffer[1] = MORE;
            buffer[i] = '\0';
            sendMessage(sockfd, buffer, i);
            i = 2;
        }
    }
    buffer[1] = LAST;
    buffer[--i] = '\0';
    printf("%s\n", buffer + 2);
    sendMessage(sockfd, buffer, i + 1);

    destroyList(head);
    closedir(dir);
    return 1;
}

#endif