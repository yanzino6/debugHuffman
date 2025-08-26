#include "arvore.h"
#include "lista.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

#define ALTURA_MAX 256

// Protótipos das funções
void calculaFrequencias(const char* nomeArquivo, unsigned long long int* arrayFrequencias);
void gerarDicionario(char* dicionario[ALTURA_MAX], Arvore* raiz, unsigned long long int* frequencias);
void preencherDicionarioRecursivo(Arvore* no, char* dicionario[ALTURA_MAX], char* caminhoAtual, int profundidade);
void serializarArvore(Arvore* raiz, bitmap* bm);
void compactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida, 
                     Arvore* raiz, char* dicionario[], unsigned long long int* frequencias);
/**
 * @brief Programa de compactação por Huffman.
 * @details Fluxo: calcula frequências; monta lista ordenada; constrói árvore; gera dicionário; serializa árvore;
 *          codifica entrada; grava cabeçalho e bitmaps no arquivo <entrada>.comp.
 * @param argc Espera 2 argumentos.
 * @param argv argv[1] = caminho do arquivo de entrada.
 * @return 0 em sucesso; 1 em erro de uso; aborta em erros de E/S.
 */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: ./compacta <arquivo_entrada>\n");
        return 1;
    }

    const char* nomeArquivo = argv[1];
    unsigned long long int frequencias[256] = {0};
    
    // Etapa 1: Calcular frequências
    calculaFrequencias(nomeArquivo, frequencias);
    
    unsigned long long int totalCaracteres = 0;
    for (int i = 0; i < 256; i++) {
        totalCaracteres += frequencias[i];
    }

    // Etapa 2: Criar lista ordenada com os nós folhas
    Lista *listaHuffman = criaListaVazia();
    int caracteresDistintos = 0;
    
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            Arvore* no_folha = criaArvore((unsigned char)i, frequencias[i], NULL, NULL);
            listaHuffman = insereOrdenado(listaHuffman, no_folha);
            caracteresDistintos++;
        }
    }
    
    // Caso especial: arquivo com apenas 1 tipo de caractere
    if (caracteresDistintos == 1) {
        // Adiciona um nó fictício com frequência 0
        unsigned char charFicticio = 0;
        // Encontra um caractere não usado
        for (int i = 0; i < 256; i++) {
            if (frequencias[i] == 0) {
                charFicticio = (unsigned char)i;
                break;
            }
        }
        Arvore* no_ficticio = criaArvore(charFicticio, 0, NULL, NULL);
        listaHuffman = insereOrdenado(listaHuffman, no_ficticio);
    }
    
    // Etapa 3: Construir a árvore de Huffman
    while (!verificaListaUmElemento(listaHuffman)) {
        Arvore* no1 = removePrimeiroLista(&listaHuffman);
        Arvore* no2 = removePrimeiroLista(&listaHuffman);
        
        unsigned long long int freq_pai = frequenciaArvore(no1) + frequenciaArvore(no2);
        Arvore* no_interno = criaArvore('\0', freq_pai, no1, no2);
        listaHuffman = insereOrdenado(listaHuffman, no_interno);
    }

    Arvore* arvoreHuffman = removePrimeiroLista(&listaHuffman);

    // Etapa 4: Gerar dicionário de códigos
    char* dicionario[ALTURA_MAX] = {NULL};
    gerarDicionario(dicionario, arvoreHuffman, frequencias);

    // Etapa 5: Compactar o arquivo
    char nomeArquivoSaida[1024];
    snprintf(nomeArquivoSaida, sizeof(nomeArquivoSaida), "%s.comp", nomeArquivo);
    
    compactarArquivo(nomeArquivo, nomeArquivoSaida, arvoreHuffman, dicionario, frequencias);

    // Estatísticas
    // printf("\n=== COMPACTAÇÃO CONCLUÍDA ===\n");
    // printf("Arquivo original: %s\n", nomeArquivo);
    // printf("Arquivo compactado: %s\n", nomeArquivoSaida);
    // printf("Caracteres distintos: %d\n", caracteresDistintos);
    // printf("Total de caracteres: %llu\n", totalCaracteres);
    
    // Limpeza de memória
    liberaLista(listaHuffman);
    liberaArvore(arvoreHuffman);
    
    for(int i = 0; i < ALTURA_MAX; i++) {
        if(dicionario[i] != NULL) {
            free(dicionario[i]);
        }
    }
    
    return 0;
}
/**
 * @brief Varre o arquivo e acumula em @p arrayFrequencias a contagem por byte (0..255).
 * @param nomeArquivo Caminho do arquivo a ler (modo binário).
 * @param arrayFrequencias Vetor de 256 posições (unsigned long long) inicializado com zeros.
 */

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
        fclose(arquivo);
        exit(1);
    }

    fclose(arquivo);
}
/**
 * @brief Cria o dicionário de códigos binários para cada byte presente.
 * @details Caso a árvore possua uma única folha (apenas um símbolo), o símbolo recebe código "0" e um nó
 *          fictício é criado no compressor para viabilizar a codificação.
 * @param dicionario Vetor de 256 ponteiros para strings (serão alocadas com strdup).
 * @param raiz Raiz da árvore de Huffman.
 * @param frequencias Vetor de frequências para filtrar símbolos inexistentes.
 */

void gerarDicionario(char* dicionario[], Arvore* raiz, unsigned long long int* frequencias) {
    // Caso especial: árvore com apenas um nó (arquivo com 1 caractere único)
    if (ehFolha(raiz)) {
        unsigned char c = caractereArvore(raiz);
        dicionario[c] = strdup("0");  // Código arbitrário "0"
        return;
    }
    
    char caminhoAtual[ALTURA_MAX];
    preencherDicionarioRecursivo(raiz, dicionario, caminhoAtual, 0);
}
/**
 * @brief Percorre a árvore (pré-ordem) acumulando '0' (esq) e '1' (dir) até folhas.
 * @param no Nó atual.
 * @param dicionario Vetor de 256 strings a preencher.
 * @param caminhoAtual Buffer temporário (stack) contendo o caminho acumulado.
 * @param profundidade Posição atual no @p caminhoAtual.
 */

void preencherDicionarioRecursivo(Arvore* no, char* dicionario[], char* caminhoAtual, int profundidade) {
    if (no == NULL) {
        return;
    }

    if (ehFolha(no)) {
        unsigned char c = caractereArvore(no);
        caminhoAtual[profundidade] = '\0';
        
        // Só adiciona ao dicionário se o caractere existe no arquivo
        // (evita adicionar o nó fictício)
        if (profundidade > 0 || caminhoAtual[0] != '\0') {
            dicionario[c] = strdup(caminhoAtual);
        }
        return;
    }

    // Navega para a esquerda com '0'
    caminhoAtual[profundidade] = '0';
    preencherDicionarioRecursivo(getEsq(no), dicionario, caminhoAtual, profundidade + 1);

    // Navega para a direita com '1'
    caminhoAtual[profundidade] = '1';
    preencherDicionarioRecursivo(getDir(no), dicionario, caminhoAtual, profundidade + 1);
}
/**
 * @brief Serializa a árvore em pré-ordem no bitmap.
 * @details Protocolo: 1 bit = 1 para folha + 8 bits do caractere; 0 para nó interno.
 * @param raiz Raiz da árvore.
 * @param bm Bitmap de saída.
 */

void serializarArvore(Arvore* raiz, bitmap* bm) {
    if (raiz == NULL) {
        return;
    }

    if (ehFolha(raiz)) {
        // Bit 1 indica folha
        bitmapAppendLeastSignificantBit(bm, 1);
        
        // Escreve o caractere (8 bits)
        unsigned char c = caractereArvore(raiz);
        for (int i = 7; i >= 0; i--) {
            unsigned char bit = (c >> i) & 1;
            bitmapAppendLeastSignificantBit(bm, bit);
        }
    } else {
        // Bit 0 indica nó interno
        bitmapAppendLeastSignificantBit(bm, 0);
        serializarArvore(getEsq(raiz), bm);
        serializarArvore(getDir(raiz), bm);
    }
}
/**
 * @brief Gera o arquivo .comp: cabeçalho + árvore serializada + dados codificados.
 * @param nomeArquivoEntrada Caminho do arquivo original.
 * @param nomeArquivoSaida Caminho do arquivo de saída (.comp).
 * @param raiz Árvore de Huffman.
 * @param dicionario Tabela de códigos por byte.
 * @param frequencias Frequências por byte (para estimar tamanho e estatísticas).
 * @details Cabeçalho: [4 bytes: tamArvoreBits] [1 byte: bitsVálidosÚltimoByteDados].
 */

void compactarArquivo(const char* nomeArquivoEntrada, const char* nomeArquivoSaida, 
                     Arvore* raiz, char* dicionario[], unsigned long long int* frequencias) {
    
    // 1. Serializar a árvore
    bitmap* bitmapArvore = bitmapInit(256 * 9 + 256);  // Máximo: 256 folhas * 9 bits + nós internos
    serializarArvore(raiz, bitmapArvore);
    
    // 2. Calcular tamanho necessário para os dados comprimidos
    unsigned long long int tamanhoComprimidoBits = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0 && dicionario[i] != NULL) {
            tamanhoComprimidoBits += frequencias[i] * strlen(dicionario[i]);
        }
    }
    
    // 3. Criar bitmap para os dados codificados
    bitmap* bitmapDados = bitmapInit(tamanhoComprimidoBits + 8);
    
    // 4. Codificar o arquivo
    FILE* arquivoEntrada = fopen(nomeArquivoEntrada, "rb");
    if (!arquivoEntrada) {
        perror("Erro ao reabrir arquivo de entrada");
        exit(1);
    }
    
    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, arquivoEntrada) == 1) {
        char* codigo = dicionario[byte];
        if (codigo != NULL) {
            for (int i = 0; codigo[i] != '\0'; i++) {
                bitmapAppendLeastSignificantBit(bitmapDados, codigo[i] - '0');
            }
        }
    }
    fclose(arquivoEntrada);
    
    // 5. Escrever arquivo compactado
    FILE* arquivoSaida = fopen(nomeArquivoSaida, "wb");
    if (!arquivoSaida) {
        perror("Erro ao criar arquivo de saída");
        exit(1);
    }
    
    // Escrever cabeçalho
    unsigned int tamanhoArvore = bitmapGetLength(bitmapArvore);
    unsigned int tamanhoDados = bitmapGetLength(bitmapDados);
    
    fwrite(&tamanhoArvore, sizeof(unsigned int), 1, arquivoSaida);
    
    // Escreve número de bits válidos no último byte dos dados
    unsigned char bitsUltimoByte = tamanhoDados % 8;
    if (bitsUltimoByte == 0) bitsUltimoByte = 8;
    fwrite(&bitsUltimoByte, sizeof(unsigned char), 1, arquivoSaida);
    
    // Escreve a árvore serializada
    unsigned int bytesArvore = (tamanhoArvore + 7) / 8;
    fwrite(bitmapGetContents(bitmapArvore), sizeof(unsigned char), bytesArvore, arquivoSaida);
    
    // Escreve os dados codificados
    unsigned int bytesDados = (tamanhoDados + 7) / 8;
    fwrite(bitmapGetContents(bitmapDados), sizeof(unsigned char), bytesDados, arquivoSaida);
    
    fclose(arquivoSaida);
    
    // Calcular taxa de compressão
    unsigned long long int tamanhoOriginal = 0;
    for (int i = 0; i < 256; i++) {
        tamanhoOriginal += frequencias[i];
    }
    
    unsigned long long int tamanhoComprimido = 5 + bytesArvore + bytesDados;  // 5 = cabeçalho
    double taxaCompressao = ((double)(tamanhoOriginal - tamanhoComprimido) / tamanhoOriginal) * 100;
    
    printf("Tamanho original: %llu bytes\n", tamanhoOriginal);
    printf("Tamanho comprimido: %llu bytes\n", tamanhoComprimido);
    printf("Taxa de compressão: %.2f%%\n", taxaCompressao > 0 ? taxaCompressao : 0);
    
    // Liberar bitmaps
    bitmapLibera(bitmapArvore);
    bitmapLibera(bitmapDados);
}