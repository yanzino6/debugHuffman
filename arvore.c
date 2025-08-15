#include "arvore.h"
#include <stdlib.h>

struct no {
    unsigned char caractere;
    unsigned long long int frequencia;
    Arvore* esq;
    Arvore* dir;
};

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

unsigned long long int frequenciaArvore(Arvore* a) {
    if (a) {
        return a->frequencia;
    }
    return 0;
}

unsigned char caractereArvore(Arvore* a) {
    if (a) {
        return a->caractere;
    }
    return 0;
}

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

int ehFolha(Arvore* a) {
    if (a) {
        return a->esq == NULL && a->dir == NULL;
    }
    return 0;
}