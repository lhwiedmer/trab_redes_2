/***************************************************************
 *
 * Nome do arquivo: servidor_tcp.c
 *
 * Descrição: Servidor TCP simples iterativo.
 *            Recebe mensagens e envia de volta.
 *
 * Autor: Giovanni Venâncio
 * Data: 29/10/2025
 *
 * Compilação:
 *     gcc servidor_tcp.c -o servidor_tcp
 *
 * Execução:
 *     ./servidor_tcp <porta>
 *
 * Exemplo:
 *     ./servidor_tcp 8500
 *
 ***************************************************************/

#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/types.h>   // Tipos de dados (socklen_t)
#include <unistd.h>      // close, read, write

#include "../message.h"

#define TAM_MAX 1024  // Tamanho máximo do buffer de dados
#define TAM_FILA 5    // Tamanho da fila de conexões

int main(int argc, char *argv[]) {
    // Verifica uso correto
    if (argc != 2) {
        fprintf(stderr, "Uso correto: %s <porta>\n", argv[0]);
        return 1;
    }

    int sockfd, connfd;  // Descritores do socket (escuta e conexão)
    struct addrinfo
        hints,  // Define o tipo de endereço do servidor (TCP + IPv4)
        *res;
    struct sockaddr_storage cliente_addr;  // Endereço genérico do cliente
    char buffer[TAM_MAX];                  // Buffer de dados
    socklen_t addr_len;                    // Tamanho da estrutura de endereço
    char *porta = argv[1];                 // Porta do servidor

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

        printf("[SERVIDOR] Aguardando conexões...\n");

        // Aceita conexão de um cliente
        connfd = accept(sockfd, (struct sockaddr *)&cliente_addr, &addr_len);
        if (connfd < 0) {
            fprintf(stderr, "accept\n");
            continue;
        }

        printf("[SERVIDOR] Cliente conectado.\n");

        // Recebe dados do cliente
        memset(buffer, 0, sizeof(buffer));
        recv(connfd, buffer, sizeof(buffer), 0);

        printf("[SERVIDOR] Mensagem recebida: %s\n", buffer);

        // Envia a mesma mensagem de volta
        send(connfd, buffer, strlen(buffer), 0);

        // Fecha conexão com o cliente
        close(connfd);
    }

    // Fecha socket de escuta
    close(sockfd);

    return 0;
}
