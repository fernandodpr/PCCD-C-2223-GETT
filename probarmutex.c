/*ABOUT: Este es el programa creado para la primera entrega de clase por error. Consiste en un algoritmo para 1 proceso N nodos
Los nodos se definen como parámetros de entrada. Siendo tantos parámeteros como Nodos introducidos por parémetro.
IMPORTANTE: El primer parámetro siempre tiene que ser la ID del nodo
Para poder ejecutar el sript de inicialización autiomática de nodos compilar con nombre de salida multinodo.o*/

/*Contexto: Lanzamiento: En función del parámetro introducido ejecuta una función o otra para poder usar el mismo programa.
(Ahora mismo parece buena idea, igual no por mucho tiempo)*/
/*Ejemplo de lanzamiento: 
multiproceso.o RECEP ID_COLA_INTERNA ID_COLA_RED #########Lanza la aplicacioón receptor 
multiproceso.o CONSULTAS ID_COLA_INTERNA ID_COLA_RED ID_NODO [ID'S NODOS]*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef TIMER
#define TIMER
#include <time.h>
#endif
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>


#include "linkedlist.h"

//gcc -o multiproceso.o multiproceso.c network.c linkedlist.c
///////////////////////////////////
#define PROBABILIDAD_ENTRADA 0.75
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01
//#define NODOSVECINOS 10



sem_t sem_crear_hilos;

sem_t sem_consultas;
sem_t sem_SC;
sem_t sem_esperaAvisoNodos;
sem_t sem_saleSC;
sem_t sem_espera_ACK;
sem_t sem_avisar_nodos_en_espera;
sem_t sem_protec_var_estado;
sem_t sem_protec_lista;

sem_t sem_prioridades[5];

struct Proceso* cola = NULL;   // Generar una lista aleatoria de 10 elementos
struct Proceso* historial = NULL;   // Generar una lista aleatoria de 10 elementos



void * procesomutex(int * param){
    //pid_t hilo_pid = getpid();
    pid_t hilo_pid = gettid();
    
    int contadorschilo=1;
    int valorSemaforoSC;
    int valorSemaforoAvisoNodos;

    do{

        //Aleatorizar la entrada en SC
            bool aleatoriaentrada=true;
            do {

                aleatoriaentrada = (double)rand() / RAND_MAX < PROBABILIDAD_ENTRADA;
                if (!aleatoriaentrada) {
                    // Si no quiero entrar espero
                    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
                }else{}
            }while (!aleatoriaentrada); // 0 No interesado 1 SOLICITANTE

            struct Proceso yomismo;
            yomismo.idProceso=hilo_pid;
            yomismo.atendido=0;
            yomismo.creado=time(NULL);
            //yomismo.idNodo=nodos[0];
            yomismo.idNodo=1;
            yomismo.prioridad=1;
            yomismo.ticket=rand() % 20; 
            
            agregarProceso(cola, yomismo);
            
            
            ordenarCola(&cola);

            if(esIgual(cola, &yomismo)){
                //Tengo permiso para entrar en SC
                cola.ejecucion=1;
                cola.pedirPermiso=0;

            }else{
                //Me espero
                sem_wait(&sem_prioridades[yomismo.prioridad]);


            }

            //SECCION CRITICA
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            int tiempoespera=rand() % 1 + 2;
            //printf("\n%i:%i.%i [Nodo %i Hilo%i] Entra durante %i.\n",t->tm_min,t->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,tiempoespera);
            //sleep(tiempoespera); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            sleep(1);
            //usleep(500000); // Espera 500000 microsegundos (medio segundo)
            printf("%i:%i:%i.%i,%i,%i,ENTRA,%i,%i\n",t->tm_hour,t->tm_min,t->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,contadorsc,contadorschilo);
            now = time(NULL);
            struct tm *t2 = localtime(&now);
            //printf("\n%i:%i.%i [Nodo %i Hilo%i] Sale. Ha entrado este nodo: %i Ha entrado este proceso: %i\n",t2->tm_min,t2->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,contadorsc,contadorschilo);
            printf("%i:%i:%i.%i,%i,%i,SALE,%i,%i\n",t->tm_hour,t->tm_min,t->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,contadorsc,contadorschilo);
            contadorsc++;
            contadorschilo++;
            

           
            
            
            sleep(1);
    }while(1);
    
}

int main(int argc, char *argv[]) {
    initparam();
    pthread_t pthtest[10];
    for (int i =0; i<10; i++) {
        //printf("Creo hilo");
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,NULL);   
    }
   
    while (1) {
    
    }
    probarlista();
    return 0;
}



int probarlista() {
    imprimirLista("listasinordenar.txt",listaAleatoria);
    ordenarCola(&listaAleatoria);
    imprimirLista("listaordenada.txt",listaAleatoria);    
    borrarLista(&listaAleatoria);   // Liberar la memoria de la lista
    return 0;
}

void initparam(){
    //INICIALIZACIÓN DE SEMAFOROS
    sem_init(&sem_SC,0,1);
    sem_init(&sem_esperaAvisoNodos,0,1);
    sem_init(&sem_espera_ACK,0,0);
    sem_init(&sem_consultas,0,5);
    sem_init(&sem_saleSC,0,0);
    sem_init(&sem_avisar_nodos_en_espera,0,0);
    sem_init(&sem_protec_var_estado,0,1);
        sem_init(&sem_protec_lista,0,1);



}