#ifndef LISTA_H
#define LISTA_H
#include "arvore.h"

typedef struct celula Lista;

/**
 * @brief Cria uma lista vazia.
 * @return NULL, representando uma lista vazia.
 */
Lista* criaListaVazia();

/**
 * @brief Insere um nó de árvore na lista de forma ordenada (crescente por frequência).
 * * @param lista A lista onde o nó será inserido.
 * @param no_arvore O nó da árvore a ser inserido.
 * @return Um ponteiro para o início da lista atualizada.
 */
Lista *insereOrdenado(Lista *l, Arvore *a);

/**
 * @brief Remove e retorna o primeiro elemento (o de menor frequência) da lista.
 * * @param lista Ponteiro para o ponteiro da lista (para poder modificá-la).
 * @return O nó da árvore que foi removido.
 */

Arvore *removePrimeiroLista(Lista **l);

/**
 * @brief Verifica se a lista possui apenas um elemento.
 * * @param lista A lista a ser verificada.
 * @return 1 se tiver um elemento, 0 caso contrário.
 */
int verificaListaUmElemento(Lista *l);

/**
 * @brief (Função para Depuração) Imprime o conteúdo da lista.
 * * @param lista A lista a ser impressa.
 */
void imprimeLista(Lista *l);

/**
 * @brief Libera toda a memória alocada para a lista.
 * * @param lista A lista a ser liberada.
 */
void liberaLista(Lista *l);

#endif