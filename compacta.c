#include "arvore.h"
#include "lista.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

#define ALTURA_MAX 256

void calculaFrequencias(const char* nomeArquivo, unsigned long long int* arrayFrequencias);
void gerarDicionario(char* dicionario[ALTURA_MAX], Arvore* raiz);
void preencherDicionarioRecursivo(Arvore* no, char* dicionario[ALTURA_MAX], char* caminhoAtual, int profundidade);
void serializarArvore(Arvore* raiz, bitmap* bm);
void compactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida, Arvore* raiz, char* dicionario[]);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Use o formato: ./compacta <arquivo_entrada>\n");
        return 1;
    }

    const char* nomeArquivo = argv[1];
    unsigned long long int frequencias[256]={0};
    calculaFrequencias(nomeArquivo, frequencias);

    Lista *listaHuffman = criaListaVazia();
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            Arvore* no_folha = criaArvore((unsigned char)i, frequencias[i], NULL, NULL);
            listaHuffman = insereOrdenado(listaHuffman, no_folha);
        }
    }
    
    
    while (!verificaListaUmElemento(listaHuffman)) {

        Arvore* no1 = removePrimeiroLista(&listaHuffman);
        Arvore* no2 = removePrimeiroLista(&listaHuffman);

        unsigned long long int freq_pai = frequenciaArvore(no1) + frequenciaArvore(no2);
        Arvore* no_interno = criaArvore('*', freq_pai, no1, no2);
        listaHuffman = insereOrdenado(listaHuffman, no_interno);

    }

    Arvore* arvoreHuffman = removePrimeiroLista(&listaHuffman);

    char* dicionario[ALTURA_MAX] = {NULL};
    gerarDicionario(dicionario, arvoreHuffman);

    // --- PARTE NOVA: COMPACTAÇÃO ---
    // Cria o nome do arquivo de saída (ex: entrada.txt -> entrada.txt.comp)
    char nomeArquivoSaida[1024];
    strcpy(nomeArquivoSaida, nomeArquivo);
    strcat(nomeArquivoSaida, ".comp");

    // Chama a função principal de compactação
    compactarArquivo(nomeArquivo, nomeArquivoSaida, arvoreHuffman, dicionario);

    printf("Arvore Huffman criada com sucesso! Frequencia da raiz: %llu\n", frequenciaArvore(arvoreHuffman));
    
    // Libera a lista (deve estar vazia, mas por segurança)
    liberaLista(listaHuffman);
    
    // Libera a árvore
    liberaArvore(arvoreHuffman);

    for(int i = 0; i < ALTURA_MAX; i++) {
        if(dicionario[i] != NULL) {
            free(dicionario[i]);
        }
    }
    return 0;

}

void calculaFrequencias(const char* nomeArquivo, unsigned long long int* arrayFrequencias) {
    FILE* arquivo = fopen(nomeArquivo, "rb");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(1);
    }

    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, arquivo) == 1) {
        arrayFrequencias[byte]++;
    }

    if (ferror(arquivo)) {
        perror("Erro durante a leitura do arquivo");
    }

    fclose(arquivo);
}

void gerarDicionario(char* dicionario[], Arvore* raiz) {
    char caminhoAtual[ALTURA_MAX];
    preencherDicionarioRecursivo(raiz, dicionario, caminhoAtual, 0);
}

void preencherDicionarioRecursivo(Arvore* no, char* dicionario[], char* caminhoAtual, int profundidade) {
    if (no == NULL) {
        return;
    }

    // Se é uma folha, armazena o caminho (código) no dicionário
    if (ehFolha(no)) {
        unsigned char c = caractereArvore(no);
        caminhoAtual[profundidade] = '\0';
        dicionario[c] = strdup(caminhoAtual); // strdup aloca memória e copia a string
        return;
    }

    // Navega para a esquerda, adicionando '0' ao caminho
    caminhoAtual[profundidade] = '0';
    preencherDicionarioRecursivo(getEsq(no), dicionario, caminhoAtual, profundidade + 1);

    // Navega para a direita, adicionando '1' ao caminho
    caminhoAtual[profundidade] = '1';
    preencherDicionarioRecursivo(getDir(no), dicionario, caminhoAtual, profundidade + 1);
}

void serializarArvore(Arvore* raiz, bitmap* bm) {
    if (raiz == NULL) {
        return;
    }

    if (ehFolha(raiz)) {
        bitmapAppendLeastSignificantBit(bm, 1); // Marca que é folha
        unsigned char c = caractereArvore(raiz);
        // Adiciona os 8 bits do caractere
        for (int i = 7; i >= 0; i--) {
            // Pega bit a bit do caractere e adiciona ao bitmap
            unsigned char bit = (c >> i) & 1;
            bitmapAppendLeastSignificantBit(bm, bit);
        }
    } else {
        bitmapAppendLeastSignificantBit(bm, 0); // Marca que é nó interno
        serializarArvore(getEsq(raiz), bm);
        serializarArvore(getDir(raiz), bm);
    }
}

/**
 * @brief Orquestra a escrita do arquivo compactado final.
 * @param nomeArquivoEntrada Arquivo original a ser lido.
 * @param nomeArquivoSaida Arquivo .comp a ser criado.
 * @param raiz A raiz da árvore de Huffman para serialização.
 * @param dicionario O dicionário de códigos para a compactação.
 */
void compactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida, Arvore* raiz, char* dicionario[]) {
    // 1. Serializar a árvore num bitmap
    bitmap* bitmapArvore = bitmapInit(2560); 
    serializarArvore(raiz, bitmapArvore);
    printf("Fez a serializacao da arvore\n");
    // 2. Criar um bitmap para os dados codificados

    FILE* tamanhoArquivo = fopen(nomeArquivoEntrada, "rb");

    unsigned long long int tamanhoTotalBits = 0;
    unsigned char byteContador;
    while (fread(&byteContador, sizeof(unsigned char), 1, tamanhoArquivo) == 1) {
        tamanhoTotalBits += 8;
    }
    fclose(tamanhoArquivo);
    
    

    bitmap* bitmapDados = bitmapInit(tamanhoTotalBits); // 8 MB de buffer, por exemplo

    // 3. Ler o arquivo de entrada novamente e codificar os dados
    FILE* arquivoEntrada = fopen(nomeArquivoEntrada, "rb");
    if (!arquivoEntrada) {
        perror("Erro ao reabrir arquivo de entrada para compactar");
        exit(1);
    }

    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, arquivoEntrada) == 1) {
        char* codigo = dicionario[byte];
        for (int i = 0; codigo[i] != '\0'; i++) {
            bitmapAppendLeastSignificantBit(bitmapDados, codigo[i] - '0');
        }
    }
    fclose(arquivoEntrada);

    // 4. Escrever tudo no arquivo de saída
    FILE* arquivoSaida = fopen(nomeArquivoSaida, "wb");
    if (!arquivoSaida) {
        perror("Erro ao criar arquivo de saida");
        exit(1);
    }

    // Escreve primeiro a árvore serializada
    fwrite(bitmapGetContents(bitmapArvore), sizeof(unsigned char), (bitmapGetLength(bitmapArvore) + 7) / 8, arquivoSaida);
    // Escreve os dados codificados
    fwrite(bitmapGetContents(bitmapDados), sizeof(unsigned char), (bitmapGetLength(bitmapDados) + 7) / 8, arquivoSaida);

    fclose(arquivoSaida);

    // Libera os bitmaps
    bitmapLibera(bitmapArvore);
    bitmapLibera(bitmapDados);

    printf("\nArquivo compactado com sucesso em: %s\n", nomeArquivoSaida);
}