#include "arvore.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Protótipos
Arvore* desserializarArvore(bitmap* bm, unsigned int* posicao);
void descompactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida);
void decodificarDados(FILE* arquivoSaida, bitmap* bitmapDados, Arvore* raiz, unsigned int numBitsValidos);
/**
 * @brief Programa de descompactação do formato .comp gerado por compacta.c.
 * @param argc Espera 2 argumentos.
 * @param argv argv[1] = caminho do arquivo .comp.
 * @return 0 em sucesso; 1 em erro de uso/extensão; aborta em erros de E/S.
 */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: ./descompacta <arquivo.comp>\n");
        return 1;
    }
    
    const char* nomeArquivoCompactado = argv[1];
    
    // Verifica se o arquivo termina com .comp
    int len = strlen(nomeArquivoCompactado);
    if (len < 5 || strcmp(nomeArquivoCompactado + len - 5, ".comp") != 0) {
        printf("Erro: O arquivo deve ter extensão .comp\n");
        return 1;
    }
    
    // Remove a extensão .comp para gerar o nome do arquivo de saída
    char nomeArquivoSaida[1024];
    strncpy(nomeArquivoSaida, nomeArquivoCompactado, len - 5);
    nomeArquivoSaida[len - 5] = '\0';
    
    // Descompacta o arquivo
    descompactarArquivo(nomeArquivoCompactado, nomeArquivoSaida);
    
    return 0;
}
/**
 * @brief Reconstrói a árvore a partir do bitmap serializado (pré-ordem).
 * @param bm Bitmap contendo a árvore.
 * @param posicao Índice do próximo bit a ler (atualizado por referência).
 * @return Raiz da árvore reconstruída ou NULL em erro.
 */

Arvore* desserializarArvore(bitmap* bm, unsigned int* posicao) {
    // Verifica se ainda há bits para ler
    if (*posicao >= bitmapGetLength(bm)) {
        return NULL;
    }
    
    // Lê o próximo bit
    unsigned char bit = bitmapGetBit(bm, (*posicao)++);
    
    if (bit == 1) {
        // É uma folha - lê o caractere (8 bits)
        unsigned char caractere = 0;
        for (int i = 7; i >= 0; i--) {
            if (*posicao >= bitmapGetLength(bm)) {
                printf("Erro: Fim inesperado do bitmap ao ler caractere\n");
                return NULL;
            }
            unsigned char b = bitmapGetBit(bm, (*posicao)++);
            caractere |= (b << i);
        }
        // Cria nó folha (frequência não importa na descompactação)
        return criaArvore(caractere, 0, NULL, NULL);
    } else {
        // É um nó interno - desserializa recursivamente
        Arvore* esq = desserializarArvore(bm, posicao);
        Arvore* dir = desserializarArvore(bm, posicao);
        return criaArvore('\0', 0, esq, dir);
    }
}
/**
 * @brief Lê o arquivo .comp e reconstroi árvore e dados, gerando o arquivo original.
 * @param nomeArquivoEntrada Caminho do .comp.
 * @param nomeArquivoSaida Caminho do arquivo de saída (sem .comp).
 * @details Fluxo: cabeçalho → bitmap da árvore → desserialização → bitmap dos dados → decodificação.
 */

void descompactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida) {
    FILE* arquivoEntrada = fopen(nomeArquivoEntrada, "rb");
    if (!arquivoEntrada) {
        perror("Erro ao abrir arquivo compactado");
        exit(1);
    }
    
    // 1. Ler cabeçalho
    unsigned int tamanhoArvore;
    unsigned char bitsUltimoByte;
    
    if (fread(&tamanhoArvore, sizeof(unsigned int), 1, arquivoEntrada) != 1) {
        printf("Erro ao ler tamanho da árvore\n");
        fclose(arquivoEntrada);
        exit(1);
    }
    
    if (fread(&bitsUltimoByte, sizeof(unsigned char), 1, arquivoEntrada) != 1) {
        printf("Erro ao ler bits do último byte\n");
        fclose(arquivoEntrada);
        exit(1);
    }
    
    // 2. Ler a árvore serializada
    unsigned int bytesArvore = (tamanhoArvore + 7) / 8;
    unsigned char* bufferArvore = (unsigned char*)malloc(bytesArvore);
    
    if (fread(bufferArvore, sizeof(unsigned char), bytesArvore, arquivoEntrada) != bytesArvore) {
        printf("Erro ao ler árvore serializada\n");
        free(bufferArvore);
        fclose(arquivoEntrada);
        exit(1);
    }
    
    // 3. Criar bitmap da árvore
    bitmap* bitmapArvore = bitmapInit(tamanhoArvore);
    for (unsigned int i = 0; i < tamanhoArvore; i++) {
        unsigned int byteIndex = i / 8;
        unsigned int bitIndex = 7 - (i % 8);
        unsigned char bit = (bufferArvore[byteIndex] >> bitIndex) & 1;
        bitmapAppendLeastSignificantBit(bitmapArvore, bit);
    }
    free(bufferArvore);
    
    // 4. Desserializar a árvore
    unsigned int posicao = 0;
    Arvore* raiz = desserializarArvore(bitmapArvore, &posicao);
    bitmapLibera(bitmapArvore);
    
    if (raiz == NULL) {
        printf("Erro ao desserializar árvore\n");
        fclose(arquivoEntrada);
        exit(1);
    }
    
    // 5. Ler os dados comprimidos
    // Primeiro, descobre o tamanho dos dados
    long posicaoAtual = ftell(arquivoEntrada);
    fseek(arquivoEntrada, 0, SEEK_END);
    long tamanhoArquivo = ftell(arquivoEntrada);
    fseek(arquivoEntrada, posicaoAtual, SEEK_SET);
    
    unsigned int bytesDados = tamanhoArquivo - posicaoAtual;
    unsigned char* bufferDados = (unsigned char*)malloc(bytesDados);
    
    if (fread(bufferDados, sizeof(unsigned char), bytesDados, arquivoEntrada) != bytesDados) {
        printf("Erro ao ler dados comprimidos\n");
        free(bufferDados);
        liberaArvore(raiz);
        fclose(arquivoEntrada);
        exit(1);
    }
    fclose(arquivoEntrada);
    
    // 6. Criar bitmap dos dados
    unsigned int totalBitsDados = (bytesDados - 1) * 8 + bitsUltimoByte;
    bitmap* bitmapDados = bitmapInit(totalBitsDados);
    
    for (unsigned int i = 0; i < totalBitsDados; i++) {
        unsigned int byteIndex = i / 8;
        unsigned int bitIndex = 7 - (i % 8);
        unsigned char bit = (bufferDados[byteIndex] >> bitIndex) & 1;
        bitmapAppendLeastSignificantBit(bitmapDados, bit);
    }
    free(bufferDados);
    
    // 7. Decodificar e escrever arquivo de saída
    FILE* arquivoSaida = fopen(nomeArquivoSaida, "wb");
    if (!arquivoSaida) {
        perror("Erro ao criar arquivo de saída");
        bitmapLibera(bitmapDados);
        liberaArvore(raiz);
        exit(1);
    }
    
    decodificarDados(arquivoSaida, bitmapDados, raiz, totalBitsDados);
    
    // 8. Limpeza
    fclose(arquivoSaida);
    bitmapLibera(bitmapDados);
    liberaArvore(raiz);
    
}
/**
 * @brief Interpreta os bits de dados caminhando pela árvore e escreve bytes decodificados.
 * @param arquivoSaida Arquivo de saída aberto (binário).
 * @param bitmapDados Bits dos dados comprimidos.
 * @param raiz Árvore de Huffman.
 * @param numBitsValidos Número de bits válidos em bitmapDados.
 */

void decodificarDados(FILE* arquivoSaida, bitmap* bitmapDados, Arvore* raiz, unsigned int numBitsValidos) {
    // Caso especial: árvore com apenas um nó (arquivo original tinha 1 caractere único)
    if (ehFolha(raiz)) {
        unsigned char c = caractereArvore(raiz);
        // Cada bit representa uma ocorrência do caractere
        for (unsigned int i = 0; i < numBitsValidos; i++) {
            fwrite(&c, sizeof(unsigned char), 1, arquivoSaida);
        }
        return;
    }
    
    // Percorre a árvore seguindo os bits
    Arvore* noAtual = raiz;
    
    for (unsigned int i = 0; i < numBitsValidos; i++) {
        unsigned char bit = bitmapGetBit(bitmapDados, i);
        
        // Navega na árvore: 0 = esquerda, 1 = direita
        if (bit == 0) {
            noAtual = getEsq(noAtual);
        } else {
            noAtual = getDir(noAtual);
        }
        
        // Se chegou numa folha, escreve o caractere e volta para a raiz
        if (ehFolha(noAtual)) {
            unsigned char c = caractereArvore(noAtual);
            fwrite(&c, sizeof(unsigned char), 1, arquivoSaida);
            noAtual = raiz;  // Volta para a raiz para decodificar o próximo caractere
        }
    }
    
    // Verifica se terminou no meio de uma decodificação (não deveria acontecer)
    if (noAtual != raiz) {
        printf("Aviso: Decodificação terminou no meio de um caminho\n");
    }
}