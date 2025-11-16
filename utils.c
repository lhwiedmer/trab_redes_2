/**
 * @name utils.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Fonte com funções de uso geral
 */

#ifndef UTILS_C
#define UTILS_C

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>   // Informações de arquivos
#include <sys/types.h>  // Tipos de dados
#include <unistd.h>     // close, read, write

#define TAM_MAX 1024  // Tamanho máximo dos buffers

int createDir(const char* dirPath) {
    struct stat st;
    if (stat(dirPath, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (mkdir(dirPath, 0755) != 0) {
                return -1;
            }
        }
    } else {
        if (mkdir(dirPath, 0755) != 0) {
            return -1;
        }
    }
    return 1;
}

node_t* createNameList(DIR* dir, const char* dirPath) {
    char fileName[TAM_MAX];

    struct stat st;

    struct dirent* file;
    file = readdir(dir);
    node_t* head = NULL;
    node_t* tail = NULL;

    while (file) {
        snprintf(fileName, TAM_MAX, "%s%s", dirPath, file->d_name);
        if (stat(fileName, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                // Eh arquivo
                node_t* aux = createNode();
                copyContent(aux, file->d_name);

                if (!head) {
                    head = tail = aux;
                } else {
                    tail->prox = aux;
                    tail = aux;
                }
            }
        }
        file = readdir(dir);
    }
    return head;
}

#endif