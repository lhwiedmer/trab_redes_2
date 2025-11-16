/**
 * @name client.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Código do cliente em um sistema cliente-servidor de upload de
 * arquivos
 */

#include <ctype.h>       // para isanul()
#include <netdb.h>       // getaddrinfo, addrinfo
#include <netinet/in.h>  // Estruturas de endereço IP (sockaddr_in)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Funções e estruturas de socket
#include <sys/types.h>   // Tipos de dados
#include <unistd.h>      // close, read, write

#include "../message.h"

/**
 * @brief Separa a primeira parte do comando digitado
 * @param[in] inBuffer Buffer com o conteúdo do comando
 * @param[in, out] firstWord Buffer zerado em que será escrita a primeira
 * palavra do comando
 * @return -1 em caso de falha, e o primeiro índice após a primeira palavra de
 * inBuffer
 */
int getFirstWord(char *inBuffer, char *firstWord) {
    int i = 0;
    // Ignora possíveis espaços iniciais
    while ((inBuffer[i] == ' ') && (inBuffer[i] != '\0')) {
        i++;
    }
    if (inBuffer[i] == '\0') {
        return -1;
    }
    // Pega primeira parte do comando
    int j = 0;
    while ((inBuffer[i] != '\0') && (inBuffer[i] != ' ') &&
           (j < sizeof("upload") - 1)) {
        firstWord[j] = inBuffer[i];
        i++;
        j++;
    }
    firstWord[j] = '\0';
    return ++i;
}

/**
 * @brief Separa a segunda parte do comando digitado
 * @param[in] inBuffer Buffer com o conteúdo do comando
 * @param[in, out] secondWord Buffer zerado em que será escrita a segunda
 * palavra do comando
 * @param[in] i Indice após o fim da primeira palavra do comando
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int getSecondWord(char *inBuffer, char *secondWord, int i) {
    // Ignora possíveis espaços iniciais
    while ((inBuffer[i] == ' ') && (inBuffer[i] != '\0')) {
        i++;
    }
    if (inBuffer[i] == '\0') {
        return -1;
    }
    // Pega segunda parte do comando
    int j = 0;
    while ((inBuffer[i] != '\0') && (inBuffer[i] != ' ')) {
        secondWord[j] = inBuffer[i];
        i++;
        j++;
    }
    secondWord[j] = '\0';
    return 1;
}

/**
 * @brief Verifica a validade de s no padrão ID
 * @param[in] s String que será verificada
 * @return 0 caso não seja válida, 1 caso seja
 */
int idIsValid(const char *s) {
    if (!s || s[0] == '\0') {
        return 0;
    }
    if (strlen(s) > 255) {
        return 0;
    }

    for (int i = 0; s[i] != '\0'; i++) {
        if (isalnum((unsigned char)s[i])) {
            continue;
        }
        if (s[i] == '_' || s[i] == '-') {
            continue;
        }

        return 0;
    }
    return 1;
}

/**
 * @brief Pede o ID pela linha de comando e o envia
 * @param[in] sockfd Descritor de socket para a conexão
 * @return -1 em caso de exit, 1 em caso de sucesso
 */
int sendId(int sockfd) {
    char buffer[TAM_MAX];
    memset(buffer, 0, TAM_MAX);
    printf("Digite seu ID:\n> ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    while ((strcmp(buffer, "exit")) && (!idIsValid(buffer))) {
        printf(
            "ID inválido, insira um ID válido.\n"
            "Caracteres válidos são letras, números, - e _.\n"
            "O ID não pode ter mais de 255 caracteres.\n"
            "Digite exit para fechar o programa.\n> ");
        memset(buffer, 0, TAM_MAX);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
    }
    if (strcmp(buffer, "exit") == 0) {
        return -1;
    }

    unsigned char idMessage[TAM_MAX];
    memset(idMessage, 0, TAM_MAX);
    idMessage[0] = ID;
    idMessage[1] = strlen(buffer);
    memcpy(idMessage + 2, buffer, idMessage[1]);

    sendMessage(sockfd, idMessage, idMessage[1] + 2);
    return rcvMessage(sockfd, idMessage, 1);
}

void printCommands() {
    printf(
        "Comandos possíveis:\n"
        "#Imprime os comandos e suas descrições na tela\n"
        "-> help\n"
        "#Faz upload do arquivo com o nome <file> para o servidor principal e "
        "suas réplicas\n"
        "-> upload <file>\n"
        "#Imprime uma lista de todos os arquivos presentes no servidor "
        "principal na tela\n"
        "-> list\n"
        "#Fecha a conexão com o servidor principal\n"
        "-> exit\n"
        "#Fecha o servidor principal e as réplicas, assim como o cliente\n"
        "-> endall\n");
}

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
    char inBuffer[TAM_MAX];  // Buffer de leitura
    memset(inBuffer, 0, TAM_MAX);
    char *host = argv[1];   // Endereço do servidor
    char *porta = argv[2];  // Porta do servidor

    // Inicialização da estrutura hints
    memset(&hints, 0, sizeof(hints));

    // Configura os parâmetros do endereço do servidor: TCP + IPv4
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP

    // Obtém estrutura de endereço do servidor
    if (getaddrinfo(host, porta, &hints, &res) != 0) {
        fprintf(stderr, "Falha ao tentar obter a estrutura getaddrinfo.\n");
        return 1;
    }

    // Cria socket TCP
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "Falha ao tentar criarr o socket.\n");
        freeaddrinfo(res);
        return 1;
    }

    // Conecta ao servidor
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "Falha ao tentar conectar com servidor.\n");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }
    printf("Conexão criada com o servidor primário.\n");
    // Libera memória da estrutura res
    freeaddrinfo(res);

    if (sendId(sockfd) == -1) {
        close(sockfd);
        return 1;
    }

    char firstWord[sizeof("upload")];
    memset(firstWord, 0, sizeof("upload"));

    while (1) {
        // Lê comando do usuário
        printf("Digite um comando:\n> ");
        fgets(inBuffer, sizeof(inBuffer), stdin);
        inBuffer[strcspn(inBuffer, "\n")] = '\0';

        int idx = getFirstWord(inBuffer, firstWord);
        if (idx == -1) {
            printf(
                "Por favor insira um comando válido. Digite help para "
                "todas "
                "as opções.\n");
        } else if (!strcmp(firstWord, "help")) {
            // Imprime comandos
            printCommands();
            continue;
        } else if (!strcmp(firstWord, "upload")) {
            // Lê resto da entrada
            char secondWord[256];
            for (int i = 0; i < 256; i++) firstWord[i] = '\0';
            if (getSecondWord(inBuffer, secondWord, idx) == -1) {
                printf(
                    "Por favor use o comando de maneira válida. upload "
                    "<file>\n");
                continue;
            }
            int r = sendFile(sockfd, secondWord, CLIENT_SERVER);
            if (r == -1) {
                printf("Houve um erro no servidor, terminando o programa.\n");
                endAll(sockfd);
                return 1;
            } else if (r == -2) {
                printf(
                    "O arquivo requisitado não existe ou não está acessível, "
                    "por favor insira um "
                    "arquivo válido.\n");
            }
        } else if (!strcmp(firstWord, "list")) {
            // Pede lista
            if (reqList(sockfd) == -1) {
                endAll(sockfd);
                return 1;
            }
        } else if (!strcmp(firstWord, "exit")) {
            // Manda mensagem para desconecar e finaliza o programa
            unsigned char a = EXIT;
            sendMessage(sockfd, &a, 1);
            break;
        } else if (!strcmp(firstWord, "endall")) {
            // Mensagem para o server fechar
            endAll(sockfd);
            return 1;
        } else {
            printf(
                "Por favor insira um comando válido. Digite help para "
                "todas "
                "as opções.\n");
            continue;
        }
    }

    // Fecha socket
    close(sockfd);
    return 0;
}
