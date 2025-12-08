/*
 * Nome: Victória Oliveira Estrela e Heitor Gabriel Lucena Albuquerque
 * Disciplina: Sistemas Operacionais
 * Arquivo: atividade02_final.c
 * Descrição: Comparação de desempenho: Threads vs Processos.
 * O código mede separadamente o tempo de criação (overhead) e o tempo total de execução.
 */

#define _POSIX_C_SOURCE 199309L // Necessário para o clock_gettime em Linux
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // Para fork, pipe, close, write, read
#include <pthread.h>    // Biblioteca principal para manipulação de Threads
#include <sys/wait.h>   // Para wait (sincronização de processos)
#include <time.h>       // Para medição de tempo (clock_gettime)
#include <string.h>
#include <math.h>       // Para pow() e sqrt()

#define TAMANHO_LISTA 10000

// --- ESTRUTURA DE ARGUMENTOS PARA THREADS ---
// Diferente de funções normais, threads só aceitam UM argumento do tipo void*.
// Para passar a lista, o tamanho e onde guardar o resultado, criamos esta struct.
typedef struct {
    int *lista;            // Ponteiro para o array de números (Memória Compartilhada)
    int total;             // Tamanho da lista
    double *resultado_ptr; // Endereço de memória onde a thread deve escrever a resposta
} ThreadArgs;

// --- TAREFAS DAS THREADS ---

// Tarefa 1: Calcular Média
void* tarefa_media(void* args) {
    // 1. Casting de void* para ThreadArgs* para acessar os dados
    ThreadArgs *dados = (ThreadArgs*)args;
    
    double soma = 0;
    int i = 0;
    // Percorre a lista somando tudo
    while(i < dados->total){
        soma += dados->lista[i++]; 
    }
    
    // ESCREVE NA MEMÓRIA COMPARTILHADA
    // A thread escreve diretamente na variável apontada.
    *(dados->resultado_ptr) = soma / dados->total; 
    
    return NULL;
}

// Tarefa 2: Calcular Mediana
void* tarefa_mediana(void* args) {
    ThreadArgs *dados = (ThreadArgs*)args;
    
    // Como a lista já foi ordenada no main (Bubble Sort prévio),
    // o cálculo é direto (O(1)), basta pegar o elemento central.
    if(dados->total % 2 != 0){ 
        *(dados->resultado_ptr) = dados->lista[(dados->total-1)/2];
    } else {
        *(dados->resultado_ptr) = (dados->lista[dados->total/2] + dados->lista[(dados->total/2)-1])/2.0;
    }
    return NULL;
}

// Tarefa 3: Calcular Desvio Padrão
void* tarefa_desvio(void* args) {
    ThreadArgs *dados = (ThreadArgs*)args;
    
    // Passo 1: Recalcular a média localmente.
    // Calculamos a média novamente aqui para garantir independência entre as threads
    // e evitar condições de corrida esperando a outra thread terminar.
    double soma = 0;
    int i = 0;
    while(i < dados->total){
        soma += dados->lista[i++];
    }
    double media = soma / dados->total;

    // Passo 2: Calcular a soma das diferenças ao quadrado
    soma = 0;
    for(i = 0; i < dados->total; i++){
        soma += pow((dados->lista[i] - media), 2);
    }
    
    // Passo 3: Raiz quadrada da variância
    *(dados->resultado_ptr) = sqrt(soma / dados->total);
    
    return NULL;
}

// --- ORQUESTRADORES (CENÁRIOS DE TESTE) ---

// EXECUÇÃO SEQUENCIAL (Single Thread / Single Process)
void executar_sequencial(int lista[], int total, char* label){
    struct timespec inicio, fim;
    
    // INÍCIO DA MEDIÇÃO
    clock_gettime(CLOCK_MONOTONIC, &inicio); 
    
    // 1. Cálculo da Média
    double soma = 0; int i = 0;
    while(i < total) soma += lista[i++];
    double media = soma/total;
    
    // 2. Cálculo da Mediana
    double mediana;
    if(total%2 != 0) mediana = lista[(total-1)/2];
    else mediana = (lista[total/2] + lista[(total/2)-1])/2.0;
            
    // 3. Cálculo do Desvio Padrão
    soma = 0;
    for(i = 0; i < total; i++) soma += pow((lista[i]-media),2); 
    double desvio = sqrt((soma/total));
                
    // FIM DA MEDIÇÃO
    clock_gettime(CLOCK_MONOTONIC, &fim); 

    printf("\n--- %s ---\n", label);
    printf("Media: %.2f | Mediana: %.2f | Desvio: %.2f\n", media, mediana, desvio);
    
    // Cálculo do tempo total
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    printf("Tempo de Criacao:  0.000000 s (N/A)\n");
    printf("Tempo Total:       %lf s\n", tempo);
}

// EXECUÇÃO MULTITHREAD (Paralelismo com Pthreads)
void executar_multithread(int lista[], int total){
    struct timespec inicio, apos_criacao, fim;
    
    // Variáveis locais que servirão de memória compartilhada para retorno
    double media, mediana, desvio;
    
    pthread_t t1, t2, t3; 
    ThreadArgs a1, a2, a3;

    clock_gettime(CLOCK_MONOTONIC, &inicio); 
    
    // FASE DE CRIAÇÃO (Overhead)
    // Prepara os argumentos e dispara as 3 threads
    a1.lista = lista; a1.total = total; a1.resultado_ptr = &media;
    pthread_create(&t1, NULL, tarefa_media, (void*)&a1);

    a2.lista = lista; a2.total = total; a2.resultado_ptr = &mediana;
    pthread_create(&t2, NULL, tarefa_mediana, (void*)&a2);

    a3.lista = lista; a3.total = total; a3.resultado_ptr = &desvio;
    pthread_create(&t3, NULL, tarefa_desvio, (void*)&a3);

    // Mede o tempo gasto apenas para criar as threads
    clock_gettime(CLOCK_MONOTONIC, &apos_criacao); 

    // SINCRONIZAÇÃO (JOIN)
    // Aguarda todas as threads terminarem antes de prosseguir
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &fim); 

    printf("\n--- 2. MULTI THREAD (3 Threads) ---\n");
    printf("Media: %.2f | Mediana: %.2f | Desvio: %.2f\n", media, mediana, desvio);
    
    // Cálculos de tempo (Criação vs Total)
    double t_criacao = (apos_criacao.tv_sec - inicio.tv_sec) + (apos_criacao.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    double t_total   = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    
    printf("Tempo de Criacao:  %lf s\n", t_criacao);
    printf("Tempo Total:       %lf s\n", t_total);
}

// EXECUÇÃO MULTIPROCESSO (Paralelismo com fork/pipes)
void executar_multiprocesso(int lista[], int total){
    struct timespec inicio, apos_criacao, fim;
    
    // Pipes para comunicação entre processos (IPC).
    // Processos têm memória isolada, então usamos pipes para enviar resultados ao pai.
    // [0] = leitura, [1] = escrita.
    int p_media[2], p_mediana[2], p_desvio[2];
    
    if(pipe(p_media) < 0 || pipe(p_mediana) < 0 || pipe(p_desvio) < 0) return;
    
    clock_gettime(CLOCK_MONOTONIC, &inicio); 
    
    // FASE DE CRIAÇÃO (FORK)
    
    // Processo Filho 1: Média
    if (fork() == 0) { 
        close(p_media[0]); // Fecha leitura (filho só escreve)
        
        // Cálculo na memória do filho (cópia)
        double soma = 0; int i = 0; while(i < total) soma += lista[i++];
        double res = soma/total;
        
        // Envia resultado para o pai
        write(p_media[1], &res, sizeof(double)); 
        close(p_media[1]); 
        exit(0); // Filho encerra aqui
    }

    // Processo Filho 2: Mediana
    if (fork() == 0) { 
        close(p_mediana[0]); 
        double res;
        // Acesso direto à lista (cópia herdada do pai)
        if(total%2 != 0) res = lista[(total-1)/2]; 
        else res = (lista[total/2] + lista[(total/2)-1])/2.0;
        
        write(p_mediana[1], &res, sizeof(double)); 
        close(p_mediana[1]); 
        exit(0);
    }

    // Processo Filho 3: Desvio Padrão
    if (fork() == 0) { 
        close(p_desvio[0]); 
        double soma = 0; int i = 0; while(i < total) soma += lista[i++];
        double med = soma/total;
        
        soma = 0; for(i=0; i<total; i++) soma += pow((lista[i]-med),2);
        double res = sqrt(soma/total);
        
        write(p_desvio[1], &res, sizeof(double)); 
        close(p_desvio[1]); 
        exit(0);
    }

    // Mede o tempo logo após disparar os 3 forks (Overhead de criação)
    clock_gettime(CLOCK_MONOTONIC, &apos_criacao); 

    // FASE DE ESPERA (WAIT)
    // O pai espera os 3 filhos terminarem para evitar processos zumbis
    wait(NULL); wait(NULL); wait(NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &fim); 

    // LEITURA DOS PIPES
    double media, mediana, desvio;
    close(p_media[1]); read(p_media[0], &media, sizeof(double));
    close(p_mediana[1]); read(p_mediana[0], &mediana, sizeof(double));
    close(p_desvio[1]); read(p_desvio[0], &desvio, sizeof(double));

    printf("\n--- 4. MULTI PROCESSO (3 Processos) ---\n");
    printf("Media: %.2f | Mediana: %.2f | Desvio: %.2f\n", media, mediana, desvio);

    double t_criacao = (apos_criacao.tv_sec - inicio.tv_sec) + (apos_criacao.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    double t_total   = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
    
    printf("Tempo de Criacao:  %lf s\n", t_criacao);
    printf("Tempo Total:       %lf s\n", t_total);
}

// --- MAIN ---
int main() {
    int lista[TAMANHO_LISTA]; 
    srand(time(NULL)); 
    
    // Geração de dados aleatórios [0, 100]
    for(int i = 0; i < TAMANHO_LISTA; i++) lista[i] = rand() % 101; 
    
    int total = TAMANHO_LISTA;

    // ORDENAÇÃO PRÉVIA (BUBBLE SORT)
    // Ordenamos a lista ANTES dos testes. Isso garante que o tempo medido
    // seja apenas do cálculo estatístico e do gerenciamento de threads/processos,
    // removendo a complexidade da ordenação da medição de tempo.
    for (int i = 0; i < total - 1; i++) {       
        for (int j = 0; j < total - i - 1; j++) { 
            if (lista[j] > lista[j+1]) { 
                int temp = lista[j]; lista[j] = lista[j+1]; lista[j+1] = temp;
            }
        }
    }
    
    printf("=== COMPARATIVO DE DESEMPENHO (Lista: %d) ===\n", TAMANHO_LISTA);
    
    // 1. Single Thread
    executar_sequencial(lista, total, "1. SINGLE THREAD (Sequencial)");
    
    // 2. Multi Thread
    executar_multithread(lista, total);
    
    // 3. Single Process (Equivalente ao sequencial)
    executar_sequencial(lista, total, "3. SINGLE PROCESS (Sequencial)"); 
    
    // 4. Multi Process
    executar_multiprocesso(lista, total);
    
    puts("\nEncerrando programa.");
    return 0;
}
