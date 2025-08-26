#ifndef ARVORE_H
#define ARVORE_H

typedef struct no Arvore;

/**
 * @brief Cria um nó da árvore de Huffman.
 * @param caractere Byte armazenado quando for folha (ignorado para nós internos).
 * @param frequencia Soma das frequências do(s) filho(s) ou do próprio caractere.
 * @param esq Ponteiro para filho esquerdo (pode ser NULL).
 * @param dir Ponteiro para filho direito (pode ser NULL).
 * @return Ponteiro para o nó criado (Arvore*). Retorna NULL em falta de memória.
 */
Arvore* criaArvore(unsigned char caractere, unsigned long long int frequencia, Arvore* esq, Arvore* dir);

unsigned long long int frequenciaArvore(Arvore* a);

unsigned char caractereArvore(Arvore* a);

/**
 * @brief Libera recursivamente toda a árvore.
 * @param a Raiz da árvore (pode ser NULL).
 * @post Toda a memória alocada para os nós é liberada.
 */
void liberaArvore(Arvore* a);

Arvore *getEsq(Arvore* a);
Arvore *getDir(Arvore* a);

/**
 * @brief Indica se o nó é folha (sem filhos).
 * @param a Nó alvo.
 * @return 1 se folha; 0 caso contrário ou @p a == NULL.
 */
int ehFolha(Arvore* a);

#endif