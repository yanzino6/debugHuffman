#ifndef ARVORE_H
#define ARVORE_H

typedef struct no Arvore;

Arvore* criaArvore(unsigned char caractere, unsigned long long int frequencia, Arvore* esq, Arvore* dir);

unsigned long long int frequenciaArvore(Arvore* a);

unsigned char caractereArvore(Arvore* a);

void liberaArvore(Arvore* a);

Arvore *getEsq(Arvore* a);
Arvore *getDir(Arvore* a);

int ehFolha(Arvore* a);

#endif