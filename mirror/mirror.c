/**
 * @name mirror.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Código do mirror em um sistema cliente-servidor de upload de
 * arquivos
 */

#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/types.h>   // Tipos de dados (socklen_t)
#include <unistd.h>      // close, read, write

#include "../message.h"
#include "../utils.h"

#define TAM_MAX 1024  // Tamanho máximo do buffer de dados
#define TAM_FILA 5    // Tamanho da fila de conexões

#define TEMP

int main(int argc, char *argv[]) {
    // Verifica uso correto
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s <porta> <dir>\n", argv[0]);
        return 1;
    }

#ifdef TEMP
    if (chdir("/tmp") != 0) {
        perror("chdir failed");
        return 1;
    }
#endif

    int sockfd, connfd;  // Descritores do socket (escuta e conexão)
    struct addrinfo
        hints,  // Define o tipo de endereço do servidor (TCP + IPv4)
        *res;
    struct sockaddr_storage cliente_addr;  // Endereço genérico do cliente
    unsigned char buffer[TAM_MAX];         // Buffer de dados
    socklen_t addr_len;                    // Tamanho da estrutura de endereço
    char *porta = argv[1];                 // Porta do servidor
    char dir[TAM_MAX];
    strcpy(dir, argv[2]);

    // Cria diretório files/ se ainda não existe
    createDir(dir);

    // Inicialização da estrutura hints
    memset(&hints, 0, sizeof(hints));

    // Configura os parâmetros do endereço do servidor: TCP + IPv4
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;      // Escuta em todas as interfaces

    // Obtém estrutura de endereço do servidor
    if (getaddrinfo(NULL, porta, &hints, &res) != 0) {
        fprintf(stderr, "getaddrinfo\n");
        return 1;
    }

    // Cria socket TCP
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "socket\n");
        freeaddrinfo(res);
        return 1;
    }

    // Bind do socket ao endereço local retornado
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "bind\n");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    // Libera memória da estrutura res
    freeaddrinfo(res);

    // Passa o socket para estado de escuta
    if (listen(sockfd, TAM_FILA) < 0) {
        fprintf(stderr, "listen\n");
        close(sockfd);
        return 1;
    }
    // Loop principal, aceita conexões iterativamente
    while (1) {
        addr_len = sizeof(cliente_addr);

        printf("Aguardando conexões...\n");

        // Aceita conexão de um cliente
        connfd = accept(sockfd, (struct sockaddr *)&cliente_addr, &addr_len);
        if (connfd < 0) {
            fprintf(stderr, "accept\n");
            continue;
        }

        printf("Conexão com servidor principal estabelecida.\n");

        memset(buffer, 0, TAM_MAX);
        char dirPath[TAM_MAX];

        char *id = rcvId(connfd, dirPath, dir, TAM_MAX - 1);

        printf("Aguardando mensagens...\n");
        rcvMessage(connfd, buffer, TAM_MAX);
        if (buffer[0] == FILEINFO) {
            printf("Operação solicitada: upload\n");
            if (rcvFile(connfd, buffer, dirPath) == -1) {
                unsigned char a = ERROR;
                sendMessage(connfd, &a, 1);
            }
            buffer[0] = OK;
            sendMessage(connfd, buffer, 1);
        } else if (buffer[0] == END) {
            printf("Operação solicitada: endall\n");
            close(connfd);
            close(sockfd);
            return 0;
        } else {
            printf("Operação desconhecida\n");
            continue;
        }
        // Fecha conexão com o cliente
        close(connfd);
        free(id);
    }

    // Fecha socket de escuta
    close(sockfd);

    return 0;
}
