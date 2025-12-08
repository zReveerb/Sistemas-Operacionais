#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <math.h>



void processo_triplo(int lista[],int total){
    
    struct timespec inicio, fim;
    double tempo_gasto;
    
    int pipe_media[2]; //Pipes para cada processo executar uma tarefa e enviar 
    int pipe_mediana[2]; //O resultado para o processo pai
    int pipe_desvio[2];
    
    pipe(pipe_media); //Crio os pipes
    pipe(pipe_mediana);
    pipe(pipe_desvio);
    
    clock_gettime(CLOCK_MONOTONIC, &inicio); //Inicio o meu cronômetro
    
    pid_t pid_p1 = fork(); //Faço o fork no processo pai P1
    if (pid_p1 == 0) { //Caso o retorno de fork() seja 0, eu estou no processo filho F1
        close(pipe_media[0]); // Fecha leitura (filho só escreve)
        int soma = 0, i =0;
        while(i < total){
            soma += lista[i++]; //Somo todos os elementos da lista e divido pelo total de elementos
        }                       //Para eu obter a média da lista
        float media = (1.0*soma)/total;
        write(pipe_media[1], &media, sizeof(media)); //Mando para o pipe_media
        
        close(pipe_media[1]); // Fecha escrita e sai
        exit(0);
    }else { //Caso não seja 0, continuo no processo pai P1
        pid_t pid_f2 = fork(); //Faço o fork no processo pai P1
        if (pid_f2 == 0) { //Caso o retorno de fork seja 0, eu estou no processo filho F2
            close(pipe_mediana[0]); //Fecha leitura (filho só escreve)
            float mediana;
            if(total%2 != 0){ //Calculo da mediana para listas com N impar e N par 
                mediana = lista[(total-1)/2];
            }else{
                mediana = (lista[total/2] + lista[(total/2)-1])/2.0;
            }
            write(pipe_mediana[1], &mediana, sizeof(mediana)); //Mando resultado para o pipe_mediana
            close(pipe_mediana[1]); //Fecha escrita e sai
            exit(0);
        }else { //Caso não seja 0, continuo no processo pai P1
            pid_t pid_f3 = fork(); //Faço outro fork no processo pai P1
            if (pid_f3 == 0) { //Caso o retorno do fork seja 0, eu estou no processo filho F3
                close(pipe_desvio[0]); //Fecha leitura (Filho só escreve)
                double soma = 0; //Para obter o desvio padrão primeiramente
                int i = 0;
                while(i < total){   //Preciso obter a média da lista
                    soma += lista[i++];
                }
                float media2 = (1.0*soma)/total;
                i = 0, soma = 0;
                for(i = 0; i < total; i++){ //Após obter a média faço o cálculo do desvio padrão
                    soma += pow((lista[i]-media2),2); 
                }
                float desvio = sqrt((soma/total));
                write(pipe_desvio[1], &desvio, sizeof(desvio)); //Mando o resultado para o pipe_desvio
                close(pipe_desvio[1]);
                exit(0);
            }
        }
    }
    wait(NULL); //Espero todos os processos terminarem para 
    wait(NULL); //Parar a contagem do meu cronômetro
    wait(NULL);
    clock_gettime(CLOCK_MONOTONIC, &fim); //Paro o meu cronômetro
    float res_media, res_mediana, res_desvio;

    // Fecha os lados de escrita (pai só lê)
    close(pipe_media[1]);
    close(pipe_mediana[1]);
    close(pipe_desvio[1]);

    // Lê os dados dos pipes
    read(pipe_media[0], &res_media, sizeof(res_media));
    read(pipe_mediana[0], &res_mediana, sizeof(res_mediana));
    read(pipe_desvio[0], &res_desvio, sizeof(res_desvio));

    // Agora sim, o Pai tem todos os dados para printar junto
    printf("\n=== RESULTADOS FINAIS (Processo Pai) EM MULTIPROCESSAMENTO ===\n");
    printf("Media:         %.2f\n", res_media);
    printf("Mediana:       %.2f\n", res_mediana);
    printf("Desvio Padrao: %.2f\n", res_desvio);
    

    tempo_gasto = (fim.tv_sec - inicio.tv_sec) + 
                  (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0; //Calculo o tempo gasto em segundos
    printf("Tempo de execucao: %lf segundos\n", tempo_gasto);
}

void processo_unico(int lista[], int total){
    
    struct timespec inicio, fim;
    double tempo_gasto;
    
    
    clock_gettime(CLOCK_MONOTONIC, &inicio); //Inicio o meu cronômetro
    
    double soma = 0;
    int i = 0;
    while(i < total){
        soma += lista[i++]; //Somo todos os elementos da lista e divido pelo total de elementos
        }                       //Para eu obter a média da lista
    float media = (1.0*soma)/total;

        
    
    float mediana;
    if(total%2 != 0){ //Calculo da mediana para listas com N impar e N par 
        mediana = lista[(total-1)/2];
    }else{
        mediana = (lista[total/2] + lista[(total/2)-1])/2.0;
        }
            
    
    soma = 0;
    for(int i = 0; i < total; i++){ //Após obter a média faço o cálculo do desvio padrão
        soma += pow((lista[i]-media),2); 
    }
    float desvio = sqrt((1.0*soma/total));
                
    clock_gettime(CLOCK_MONOTONIC, &fim); //Paro o meu cronômetro


    //Printo os resultados
    printf("\n=== RESULTADOS FINAIS (Processo Pai) EM PROCESSO ÚNICO ===\n");
    printf("Media:         %.2f\n", media);
    printf("Mediana:       %.2f\n", mediana);
    printf("Desvio Padrao: %.2f\n", desvio);
    
    tempo_gasto = (fim.tv_sec - inicio.tv_sec) + 
                  (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0; //Calculo o tempo gasto em segundos
    printf("Tempo de execucao: %lf segundos\n", tempo_gasto);
}
int main() {
    int lista[10000]; //Separo 10000 espacos de memória para a minha lista de 100 
    srand(time(NULL)); //NUmeros aleatorios entre [0,100]
    for(int i = 0; i < 10000; i++){ //Preencho essa lista com os numeros aleatorios
        int aleatorio = rand() % 101;
        lista[i] = aleatorio;
    }
    
    int total = sizeof(lista)/sizeof(lista[0]);
    for (int i = 0; i < total - 1; i++) {       //Ordeno a lista com bubble sort
            for (int j = 0; j < total - i - 1; j++) { //Pois a lista precisa estar ordenada
                if (lista[j] > lista[j+1]) { //Para o calculo da mediana
                    // Troca os elementos de posição
                    int temp = lista[j];
                    lista[j] = lista[j+1];
                    lista[j+1] = temp;
                }
            }
        }
    
    puts("Iniciando execucao para um unico processo:");
    processo_unico(lista, total);
    puts("\nIniciando execucao para tres processos diferentes:");
    processo_triplo(lista, total);
    puts("Encerrando o programa...");
    return 0;
}
// Created by heitor on 07/12/2025.
//