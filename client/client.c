/***************************************************************
 *
 * Nome do arquivo: cliente_tcp.c
 *
 * Descrição: Cliente TCP simples.
 *            Envia uma mensagem e recebe de volta.
 *
 * Autor: Giovanni Venâncio
 * Data: 29/10/2025
 *
 * Compilação:
 *     gcc cliente_tcp.c -o cliente_tcp
 *
 * Execução:
 *     ./cliente_tcp <host> <porta>
 *
 * Exemplo:
 *     ./cliente_tcp 127.0.1.1 8500
 *
 ***************************************************************/

#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/types.h>   // Tipos de dados
#include <unistd.h>      // close, read, write

#define TAM_MAX 1024  // Tamanho máximo da mensagem

int main(int argc, char *argv[]) {
    // Verifica uso correto
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s <host> <porta>\n", argv[0]);
        return 1;
    }

    int sockfd;  // Descritor do socket
    struct addrinfo
        hints,  // Define o tipo de endereço do servidor (TCP + IPv4)
        *res;
    char buffer[TAM_MAX];   // Buffer de dados
    char *host = argv[1];   // Endereço do servidor
    char *porta = argv[2];  // Porta do servidor

    // Inicialização da estrutura hints
    memset(&hints, 0, sizeof(hints));

    // Configura os parâmetros do endereço do servidor: TCP + IPv4
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP

    // Obtém estrutura de endereço do servidor
    if (getaddrinfo(host, porta, &hints, &res) != 0) {
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

    // Conecta ao servidor
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "connect\n");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }
    printf("Conexão criada com o servidor primário.\n");
    // Libera memória da estrutura res
    freeaddrinfo(res);
    // Lê mensagem do usuário
    printf("Escreva o seu username:\n>");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    // Envia mensagem para o servidor
    send(sockfd, buffer, strlen(buffer), 0);

    // Recebe resposta do servidor
    recv(sockfd, buffer, sizeof(buffer), 0);

    printf("[CLIENTE] Resposta recebida: %s\n", buffer);

    // Fecha socket
    close(sockfd);
    return 0;
}
