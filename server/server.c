/**
 * @name client.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Código do servidor em um sistema cliente-servidor de upload de
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
        fprintf(stderr, "Uso correto: %s <porta> <arquivo de configuração>\n",
                argv[0]);
        return 1;
    }

    FILE *arq = fopen(argv[2], "r");
    if (!arq) {
        printf("Falha ao tentar abrir arquivo de configuração\n");
        return 1;
    }

    struct addrinfo
        hints,  // Define o tipo de endereço do servidor (TCP + IPv4)
        *res;
    int n;
    fscanf(arq, "%3d", &n);
    int sockBuff[n];
    char hostM[50];
    char portM[50];
    for (int i = 0; i < n; i++) {
        fscanf(arq, "%49s", hostM);
        fscanf(arq, "%49s", portM);

        // Inicialização da estrutura hints
        memset(&hints, 0, sizeof(hints));

        // Configura os parâmetros do endereço do servidor: TCP + IPv4
        hints.ai_family = AF_INET;        // IPv4
        hints.ai_socktype = SOCK_STREAM;  // TCP

        // Obtém estrutura de endereço do servidor
        if (getaddrinfo(hostM, portM, &hints, &res) != 0) {
            fprintf(stderr, "Falha ao tentar obter a estrutura getaddrinfo.\n");
            return 1;
        }

        // Cria socket TCP
        sockBuff[i] =
            socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockBuff[i] < 0) {
            fprintf(stderr, "Falha ao tentar criarr o socket.\n");
            freeaddrinfo(res);
            return 1;
        }

        // Conecta ao servidor
        if (connect(sockBuff[i], res->ai_addr, res->ai_addrlen) < 0) {
            fprintf(stderr, "Falha ao tentar conectar com servidor.\n");
            close(sockBuff[i]);
            freeaddrinfo(res);
            return 1;
        }
    }

    fclose(arq);

#ifdef TEMP
    if (chdir("/tmp") != 0) {
        perror("chdir failed");
        return 1;
    }
#endif

    int sockfd, connfd;  // Descritores do socket (escuta e conexão)

    struct sockaddr_storage cliente_addr;  // Endereço genérico do cliente
    unsigned char buffer[TAM_MAX];         // Buffer de dados
    socklen_t addr_len;                    // Tamanho da estrutura de endereço
    char *porta = argv[1];                 // Porta do servidor

    // Cria diretório files/ se ainda não existe
    createDir(FILE_DIR);

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
    int count = 0;
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

        printf("Conexão com cliente [%d] estabelecida.\n", count);

        memset(buffer, 0, TAM_MAX);

        // Receber ID do cliente
        rcvMessage(connfd, buffer, TAM_MAX);
        for (int i = 0; i < n; i++) {
            sendMessage(sockBuff[i], buffer, TAM_MAX);
        }
        char id[256];
        for (int i = 0; i < buffer[1]; i++) {
            id[i] = buffer[i + 2];
        }
        id[buffer[1]] = '\0';

        printf("Cliente identificado como: %s\n", id);

        id[buffer[1]] = '/';
        id[buffer[1] + 1] = '\0';

        // Monta a path para o diretório do cliente
        char dirPath[TAM_MAX];
        strcpy(dirPath, FILE_DIR);
        strcat(dirPath, id);

        unsigned char a = OK;

        sendMessage(connfd, &a, 1);

        // Loop para lidar com mensagens vindas do cliente conectado
        while (1) {
            printf("Aguardando mensagens...\n");
            memset(buffer, 0, sizeof(buffer));
            recv(connfd, buffer, sizeof(buffer), 0);
            if (buffer[0] == FILEINFO) {
                printf("Operação solicitada: upload\n");
                if (rcvFile(connfd, buffer, dirPath) == -1) {
                    unsigned char a = ERROR;
                    sendMessage(connfd, &a, 1);
                }
                printf("Iniciando processo de replicação...\n");
                unsigned char correct = 0;
                unsigned char a;
                for (int i = 0; i < n; i++) {
                    sendFile(sockBuff[i], (char *)buffer, SERVER_MIRROR);
                    if (rcvMessage(sockBuff[i], &a, 1) == -1) {
                        printf("[Réplica %d] ERRO", i + 1);
                        continue;
                    }
                    correct++;
                    printf("[Réplica %d] OK\n", i + 1);
                }
                buffer[0] = OK;
                buffer[1] = correct;
                sendMessage(connfd, buffer, n);
                memset(buffer, 0, TAM_MAX);
            } else if (buffer[0] == LISTREQ) {
                printf("Operação solicitada: list\n");
                if (sendList(connfd, dirPath) == -1) {
                    unsigned char a = ERROR;
                    sendMessage(connfd, &a, 1);
                }
            } else if (buffer[0] == EXIT) {
                printf("Operação solicitada: exit\n");
                break;
            } else if (buffer[0] == END) {
                printf("Operação solicitada: endall\n");
                for (int i = 0; i < n; i++) {
                    endAll(sockBuff[i]);
                }
                close(connfd);
                close(sockfd);
                return 0;
            } else {
                printf("Operação desconhecida\n");
            }
        }
        // Fecha conexão com o cliente
        close(connfd);
        count++;
    }

    // Fecha socket de escuta
    close(sockfd);

    return 0;
}
