#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    srand(getpid());

    printf("[Proceso %d] -> Esperando...\n", getpid());
    sleep(rand()%10+1);
    printf("[Proceso %d] -> Intentando acceder a mi Seccion Critica\n", getpid());
    sleep(rand()%10+1);
    //***************************** SECCION CRITCA ****************************
    printf("[Proceso %d] -> Dentro de mi Seccion Critica\n", getpid());
    sleep(rand()%10+1);
    printf("[Proceso %d] -> He salido de mi Seccion Critica\n", getpid());
    //*************************************************************************

    return 0;
}