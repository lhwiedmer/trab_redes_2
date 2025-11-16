/**
 * @name list.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Fonte com funções e estrutura de lista
 */

#ifndef LIST_C
#define LIST_C

#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

node_t* createNode() {
    node_t* n = malloc(sizeof(node_t));
    if (!n) {
        perror("Falha de alocação\n");
        exit(1);
    }
    n->content = malloc(NAME_SIZE);
    if (!n->content) {
        perror("Falha de alocação\n");
        exit(1);
    }
    n->size = 0;
    n->prox = NULL;
    return n;
}

void copyContent(node_t* n, const char* buffer) {
    strncpy(n->content, buffer, NAME_SIZE);
    n->size = strlen(buffer);
}

void destroyList(node_t* n) {
    while (n) {
        node_t* aux = n;
        n = n->prox;
        free(aux->content);
        free(aux);
    }
}

#endif