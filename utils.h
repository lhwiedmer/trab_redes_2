/**
 * @name utils.c
 * @author Luiz Henrique Murback Wiedmer
 * @details Header com funções de uso geral
 */

#ifndef UTILS_H
#define UTILS_H

#include <dirent.h>  // Gerenciamento de diretórios(lista de arquivos)

#include "list.h"

/**
 * @brief Verifica se um diretório existe, se não existir o cria
 * @param[in] dirPath Path do dir a ser criado
 * @attention A função não consegue criar vários diretórios aninhados
 */
int createDir(const char* dirPath);

/**
 * @brief Cria uma lista com os nomes dos arquivos em um diretório
 * @param[in, out] dir Ponteiro para o diretório a ser verificado
 * @param[in] dirPath Path do dir a ser verificado
 * @return Ponteiro para o início da lista ou NULL caso o diretório esteja vazio
 */
node_t* createNameList(DIR* dir, const char* dirPath);

#endif