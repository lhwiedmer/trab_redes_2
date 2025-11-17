/**
 * @name message.h
 * @author Luiz Henrique Murback Wiedmer
 * @details Header com os códigos de mensagem e funções tratando cada comando
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#define TAM_MAX 1024  // Tamanho máximo da mensagem

#define CLIENT_SERVER 0
#define SERVER_MIRROR 1

/**
 * @brief Código para cada tipo de mensagem
 */
typedef enum Code {
    OK,  //!< Mensagem de confirmação para que o outro lado possa continuar a
         //!< comunicação
    ID,  //!< Mensagem contendo o ID do cliente
    DATA,      //!< Mensagem enviando conteúdo de um arquivo
    FILEINFO,  //!< Mensagem enviando metadados de um aquivo
    LISTREQ,   //!< Mensagem enviando pedido da lista de arquivos
    LISTINFO,  //!< Informações da lista de arquivos
    LISTASW,   //!< Mensagem enviando parte da lista de arquivos
    EXIT,      //!< Desconecta do servidor
    END,       //!< Fecha o servidor
    ERROR      //!< Erro do lado do servidor
} code_t;

typedef enum ListMore {
    NONE,  //!< Flag dizendo que não há nenhum arquivo no servidor
    LAST,  //!< Flag dizendo que é a última mensagem daquele pedido de lista
    MORE   //!< Flag dizendo que virão mais mensagens daquele pedido de lista
} list_t;

/**
 * @brief Envia uma mensagem
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] buffer Buffer com conteúdo a ser enviado
 * @param[in] n Tamanho do conteúdo a ser enviado
 * @attention Em caso de erros, a função termina o programa
 */
void sendMessage(int sockfd, unsigned char* buffer, unsigned long n);

/**
 * @brief Recebe uma mensagem
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[out] buffer Buffer em que será escrito o conteúdo recebido
 * @param[in] n Tamanho do conteúdo a ser recebido
 * @return -1 em caso de erro do servidor, 1 em caso de sucesso
 * @attention Em caso de erros, a função termina o programa
 */
int rcvMessage(int sockfd, unsigned char* buffer, unsigned long n);

/**
 * @brief Envia uma mensagem para o servidor fechar
 * @param[in] sockfd Descritor do socket para a conexão
 */
void endAll(int sockfd);

/**
 * @brief Envia os dados de um arquivo
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] fileName Nome do arquivo a ser enviado
 * @param[in] mode Modo de uso da função, pode ser CLIENT_SERVER OU
 * SERVER_MIRROR
 * @return -1 em caso de falha, -2 caso o arquivo não exista, e 1 em caso de
 * sucesso
 */
int sendFile(int sockfd, const char* fileName, int mode);

/**
 * @brief Recebe os dados de um arquivo
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in, out] buffer Metadados do arquivo(deve ser alocado com TAM_MAX). Ao
 * final da função possui a path para o arquivo em que foi escrito o conteúdo.
 * @param[in] dirPath Path para o diretório em que será escrito o arquivo
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int rcvFile(int sockfd, unsigned char* buffer, const char* dirPath);

/**
 * @brief Pede e recebe a lista de arquivos presentes no servidor
 * @param[in] sockfd Descritor de socket para a conexão
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int reqList(int sockfd);

/**
 * @brief Envia a lista de arquivos no servidor
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] dirPath Path para o diretório em que será escrito o arquivo
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int sendList(int sockfd, const char* dirPath);

#endif