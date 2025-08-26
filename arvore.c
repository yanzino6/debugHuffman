#include "arvore.h"
#include <stdlib.h>

struct no {
    unsigned char caractere;
    unsigned long long int frequencia;
    Arvore* esq;
    Arvore* dir;
};
/**
 * @brief Cria um nó da árvore de Huffman.
 * @param caractere Byte armazenado quando for folha (ignorado para nós internos).
 * @param frequencia Soma das frequências do(s) filho(s) ou do próprio caractere.
 * @param esq Ponteiro para filho esquerdo (pode ser NULL).
 * @param dir Ponteiro para filho direito (pode ser NULL).
 * @return Ponteiro para o nó criado (Arvore*). Retorna NULL em falta de memória.
 
 */

Arvore* criaArvore(unsigned char caractere, unsigned long long int frequencia, Arvore* esq, Arvore* dir) {
    Arvore* novo_no = (Arvore*) malloc(sizeof(Arvore));
    if (novo_no) {
        novo_no->caractere = caractere;
        novo_no->frequencia = frequencia;
        novo_no->esq = esq;
        novo_no->dir = dir;
    }
    return novo_no;
}
/**
 * @brief Retorna a frequência associada ao nó.
 * @param a Nó alvo.
 * @return Frequência do nó ou 0 se @p a == NULL.
 */

unsigned long long int frequenciaArvore(Arvore* a) {
    if (a) {
        return a->frequencia;
    }
    return 0;
}
/**
 * @brief Retorna o caractere armazenado no nó (folha).
 * @param a Nó alvo.
 * @return Caractere do nó ou 0 se @p a == NULL.
 */

unsigned char caractereArvore(Arvore* a) {
    if (a) {
        return a->caractere;
    }
    return 0;
}
/**
 * @brief Libera recursivamente toda a árvore.
 * @param a Raiz da árvore (pode ser NULL).
 * @post Toda a memória alocada para os nós é liberada.
 */

void liberaArvore(Arvore* a) {
    if (a) {
        liberaArvore(a->esq);
        liberaArvore(a->dir);
        free(a);
    }
}

Arvore *getEsq(Arvore* a)
{
    return a->esq;
}
Arvore *getDir(Arvore* a)
{
    return a->dir;
}
/**
 * @brief Indica se o nó é folha (sem filhos).
 * @param a Nó alvo.
 * @return 1 se folha; 0 caso contrário ou @p a == NULL.
 */

int ehFolha(Arvore* a) {
    if (a) {
        return a->esq == NULL && a->dir == NULL;
    }
    return 0;
}