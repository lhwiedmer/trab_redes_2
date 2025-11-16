/**
 * @name list.h
 * @author Luiz Henrique Murback Wiedmer
 * @details Header com funções e estrutura de lista
 */

#ifndef LIST_H
#define LIST_H

#define NAME_SIZE 256

/**
 * @brief Nodo de uma lista
 */
typedef struct node {
    unsigned char size;  //!< Tamanho do conteúdo
    char* content;       //!< Conteúdo
    struct node* prox;
} node_t;

/**
 * @brief Aloca o espaço para um nodo
 * @return Ponteiro para nodo alocado
 * @attention Essa função termina o programa caso a alocação falhe
 */
node_t* createNode();

/**
 * @brief Copia o conteudo para o nodo
 * @param[in, out] n Nodo a receber conteúdo
 * @param[in] n Conteúdo a ser copiado
 */
void copyContent(node_t* n, const char* buffer);

/**
 * @brief Libera a memória de uma lista de nodos
 * @param[in, out] n Cabeça da lista
 */
void destroyList(node_t* n);

#endif