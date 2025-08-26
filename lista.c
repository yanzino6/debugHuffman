#include "lista.h"
#include <stdlib.h>
#include "arvore.h"
#include "stdio.h"

struct celula {
    Arvore* no_arvore;
    Lista* prox;
};

/**
 * @brief Cria uma lista vazia.
 * @return NULL, representando uma lista vazia.
 */
 Lista* criaListaVazia() {
    return NULL;
 }

 /**
  * @brief Insere um nó de árvore na lista de forma ordenada (crescente por frequência).
  * * @param lista A lista onde o nó será inserido.
  * @param no_arvore O nó da árvore a ser inserido.
  * @return Um ponteiro para o início da lista atualizada.
  */
 Lista *insereOrdenado(Lista *l, Arvore *a)
 {
    Lista* nova = (Lista*)malloc(sizeof(Lista));
    nova->no_arvore = a;
    if (l == NULL || frequenciaArvore(a) < frequenciaArvore(l->no_arvore)) {
        nova->prox = l;
        return nova;
    }
    Lista* aux = l;
    while (aux->prox != NULL && frequenciaArvore(a) > frequenciaArvore(aux->prox->no_arvore)) {
        aux = aux->prox;
    }
    nova->prox = aux->prox;
    aux->prox = nova;

    return l;
 }
 
 /**
  * @brief Remove e retorna o primeiro elemento (o de menor frequência) da lista.
  * * @param lista Ponteiro para o ponteiro da lista (para poder modificá-la).
  * @return O nó da árvore que foi removido.
  */
 
 Arvore *removePrimeiroLista(Lista **l)
 {
    if (*l == NULL) {
        return NULL;
    }
    Lista* aux = *l;
    Arvore* removido = aux->no_arvore;
    *l = aux->prox; 
    free(aux);
    return removido;
 }
 
 /**
  * @brief Verifica se a lista possui apenas um elemento.
  * * @param lista A lista a ser verificada.
  * @return 1 se tiver um elemento, 0 caso contrário.
  */
 int verificaListaUmElemento(Lista *l)
 {
    return (l != NULL && l->prox == NULL);
 }
 
 /**
  * @brief (Função para Depuração) Imprime o conteúdo da lista.
  * * @param lista A lista a ser impressa.
  */
 void imprimeLista(Lista *l)
 {
    printf("--- LISTA ORDENADA ---\n");
    Lista* p = l;
    while (p != NULL) {
        unsigned char c = caractereArvore(p->no_arvore);
        printf("Caractere: ");
        if (c >= 32 && c <= 126) {
            // Caractere imprimível
            printf("'%c' ", c);
        } else {
            // Caractere não imprimível (mostra código ASCII)
            printf("[%d] ", c);
        }
        printf("| Freq: %llu\n", frequenciaArvore(p->no_arvore));
        p = p->prox;
    }
    printf("--- FIM DA LISTA ---\n");
 }
/**
 * @brief Libera todos os nós (células) da lista.
 * @param l Cabeça da lista.
 * @post A lista é descartada; não libera as árvores apontadas.
 * @complexity O(k).
 */

 void liberaLista(Lista *l) {
    while (l != NULL) {
        Lista* aux = l;
        l = l->prox;
        free(aux);
    }
 }