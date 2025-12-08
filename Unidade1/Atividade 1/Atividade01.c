//Feito por Heitor Gabriel e Victória Oliveira
//Cria uma árvore com 1 processo pai, 2 processos filhos F1 (com processos netos N1 e N2) e F2 (com processos netos N3 e N4)
//Cada folha vai executar uma operação diferente
//Nós filhos e nó pai vão informar o seu PID

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(){
    pid_t pidp1 = fork(); //Faço o fork() e crio o processo filho f1
    if(pidp1 < 0){
        printf("Erro no processo de forking!!\n");
        exit(1);
    }
    if (pidp1 == 0){ //Se o retorno do fork() for igual a zero, me encontro no processo filho f1
        pid_t pidf1 = fork(); //Faço um fork() a partir de f1 para gerar N1
        if (pidf1 == 0){ //Me encontro no processo filho N1 caso retorno do fork seja 0

            execl("/bin/ls", "ls", "-l", NULL); //(Listar arquivos) Execução de código em N1
            exit(1); // Encerra o filho indicando erro (1) se chegar aqui
        }else{ //Caso seja diferente de zero, continuo em F1
            pid_t pidn2 = fork(); //Crio o processo filho N2 a partir de F1
            if (pidn2 == 0){ //Caso o retorno do fork seja 0, estou no processo filho N2

                execl("/bin/date", "date", NULL); //( Mostrar data) Execução de código em N2
                exit(1); // Encerra o filho indicando erro (1) se chegar aqui
            }
            wait(NULL); //Espero o processo filho N1 terminar
            wait(NULL); //Espero o processo filho N2 terminar
            printf("\nProcessos N1 e N2 terminados!\nMe encontro agora no processo F1!\n");
            printf("PID processo F1: %d\nPID processo pai de F1(P1): %d\n\n", getpid(), getppid());
            exit(0); //Finalizo o processo filho F1
        }
    }else{ //Caso o retorno do fork de pidp1 seja diferente de zero, estou em P1
        pid_t pidf2 = fork(); //Crio o processo f2 a partir de P1
        if (pidf2 == 0){ //Se o retorno do fork() for 0, estou no processo filho f2
            pid_t pidn3 = fork(); //Crio o processo filho N3 a partir de f2
            if (pidn3 == 0){  //Se o retorno do fork for 0, estou no processo filho N3

                execl("/bin/pwd", "pwd", NULL); //(Mostrar diretório atual) Execução de código em n3
                exit(1); // Encerra o filho indicando erro (1) se chegar aqui
            }else{ //Ainda estou no processo pai F2
                pid_t pidn4 = fork(); //Crio o processo filho N4 a partir de F2
                if(pidn4 == 0){ //Caso o retorno do fork seja 0, me encontro no processo N4

                    execl("/bin/ps", "ps", NULL); //(Listar processos)  Execução de código em N4
                    exit(1); // Encerra o filho indicando erro (1) se chegar aqui
                }
                wait(NULL); //Espero o processo filho N3 terminar
                wait(NULL); //Espero o processo filho N4 terminar
                printf("\nProcessos N3 e N4 terminados!\nMe encontro agora no processo F2!\n");
                printf("PID processo F2: %d\nPID processo pai de F2(P2): %d\n", getpid(), getppid());
                exit(0); //Finalizo o processo filho F2

            }
        }
        wait(NULL); //Espera o processo F1 terminar
        wait(NULL); // Espero o processo F2 terminar
        printf("\nEstou no processo pai P1!\nPID PROCESSO P1: %d\n", getpid());
    }
    printf("\nPROGRAMA ENCERRANDO... BIP BOP"); //Mensagem de encerramento do programa!
    exit(0); //Encerro o processo P1
}
