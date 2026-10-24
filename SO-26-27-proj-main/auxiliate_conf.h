#ifndef AUXILIATE_CONF__H
#define AUXILIATE_CONF__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Verifica se o ficheiro é do tipo indicado
 *
 * @param str ficheiro a analisar
 * @param ext tipo de extensão a analisar
 *
 * @return 1 se tiver a extensão pretendida devolve 0 caso contrário
 */
int verifica_extensao(const char * str, const char * ext); 

/**
 * Compara dois nomes de ficheiros com a finalidade de os ordenar alfabeticamente com o qsort
 *
 * @param a primeiro ficheiro
 * @param b segundo ficheiro
 *
 * @return 0 se os nomes forem iguais, 1 se o comprimento do nome do ficheiro a é 
 *         maior do que o comprimento do nome do ficheiro b, caso contrário devolve 
 *         um número negativo.
 */
int comparar_nomes(const void *a, const void *b);

/**
* Realiza a tarefa, assegurando sincronização entre processos.
*
* @param arg ponteiro para a estrutura Thread
* @return NULL, simbolizando assim o fim de todo o processo da lista de ficheiros
*/
void *do_thread(void *arg);

#endif