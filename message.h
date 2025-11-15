/**
 * @name message.h
 * @author Luiz Henrique Murback Wiedmer
 * @details Header com os códigos de mensagem e funções tratando cada comando
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#define FILE_DIR "files/"  // Não usar diretório aninhado
#define TAM_MAX 1024       // Tamanho máximo da mensagem

/**
 * @brief Código para cada tipo de mensagem
 */
typedef enum Code {
    OK,  //!< Mensagem de confirmação para que o outro lado possa continuar a
         //!< comunicação
    DATA,      //!< Mensagem enviando conteúdo de um arquivo
    FILEINFO,  //!< Mensagem enviando metadados de um aquivo
    LISTREQ,   //!< Mensagem enviando pedido da lista de arquivos
    LISTINFO,  //!< Informações da lista de arquivos
    LISTASW,   //!< Mensagem enviando parte da lista de arquivos
    EXIT,      //!< Desconecta do servidor
    END,       //!< Fecha o servidor
    ERROR      //!< Erro do lado do servidor
} code_t;

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
 * @brief Envia os dados de um arquivo
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] fileName Nome do arquivo a ser enviado
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int sendFile(int sockfd, const char* fileName);

/**
 * @brief Recebe os dados de um arquivo
 * @param[in] sockfd Descritor de socket para a conexão
 * @param[in] buffer Metadados do arquivo(deve ser alocado com TAM_MAX)
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int rcvFile(int sockfd, unsigned char* buffer);

/**
 * @brief Pede a lista de arquivos presentes no servidor
 * @param[in] sockfd Descritor de socket para a conexão
 * @return -1 em caso de falha, e 1 em caso de sucesso
 */
int reqList(int sockdfd);

#endif