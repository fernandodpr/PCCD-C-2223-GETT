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

struct Proceso* cola = NULL;  
struct Proceso* historial = NULL;   

void initparam();

void * procesomutex(int * param){
    //pid_t hilo_pid = getpid();
    int tiempo_proceso_inicio= time(NULL);
    pid_t hilo_pid = gettid();
    
    //int contadorschilo=1;
    //int valorSemaforoSC;
    //int valorSemaforoAvisoNodos;
    int nodos[1];
    nodos[0]=1;
    printf("Acabo de entrar en procesomutex\n");
    
    //do{

        //Aleatorizar la entrada en SC
            bool aleatoriaentrada=true;
            printf("Intentando entrar en la seccion critica...\n");
            do {

                aleatoriaentrada = (double)rand() / RAND_MAX < PROBABILIDAD_ENTRADA;
                if (!aleatoriaentrada) {
                    // Si no quiero entrar espero
                    
                    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
                }else{}
            }while (!aleatoriaentrada); // 0 No interesado 1 SOLICITANTE
            printf("Quiero entrar en SC\n");
            
            struct Proceso yomismo;

            yomismo.idProceso=0;
            yomismo.atendido=0;
            yomismo.contACK=0;
            yomismo.creado=0;
            yomismo.idNodo=nodos[0];
            yomismo.prioridad=1;
            yomismo.ticket=rand() % 20; 
            
            //Me pongo a la cola y ordeno
            sem_wait(&sem_protec_lista);
            agregarProceso(&cola, yomismo.prioridad, yomismo.ticket, yomismo.idNodo,hilo_pid,0,0,tiempo_proceso_inicio,0);
            ordenarCola(&cola);
            sem_post(&sem_protec_lista);

            if(esIgual(cola, &yomismo)){
                //Tengo permiso para entrar en SC
                printf("Tengo permisos\n");  
                cola->ejecucion=1;
                cola->pedirPermiso=0;

            }else{
                //Me espero
                printf("NO Tengo permisos\n");
                sem_wait(&sem_prioridades[yomismo.prioridad]);
                printf("He despertado!\n");



            }

            //SECCION CRITICA

            printf("Acabo de llegar a SC con Ticket: %i ",yomismo.ticket);
            int tiempo_sc_inicio= time(NULL);
            int tiempoespera=rand() % 1 + 5;
            sleep(tiempoespera);
            int tiempo_sc_fin= time(NULL);       


            agregarProceso(&cola, yomismo.prioridad, yomismo.ticket, yomismo.idNodo,hilo_pid,0,0,tiempo_proceso_inicio,tiempo_sc_fin);


            sem_wait(&sem_protec_lista);
            eliminarCabeza(&cola);
            ordenarCola(&cola);
            sem_post(&sem_protec_lista);

            if(compararIdNodo(cola,nodos[0])){
                //El sigueinte proceso está esperando en mi nodo
                printf("EL SIGUIENTE PROCESO ESTA EN MI NODO\n");
                printf("El sigueinte proceso es de prioridad %i\n",cola->prioridad);
                printf("El sigueinte proceso es tiene ticket %i\n",cola->ticket);

                sem_post(&sem_prioridades[cola->prioridad]);
                printf("Ha despertado?\n");
            }else{
                //El siguinte proceso esta esperando fuera de mi nodo
                printf("EL SIGUIENTE PROCESO NO ESTA EN MI NODO\n");

            }
           
            
            sleep(1);
        //}while(1);

    return NULL;
    
}

int main(int argc, char *argv[]) {
    printf("Hola\n");
    srand(time(NULL)); 
    initparam();
    pthread_t pthtest[10];

    for (int i =0; i<20; i++) {
        printf("Creo hilo\n");
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,NULL);   
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(pthtest[i], NULL); // Esperar a que el hilo termine
    }

   
    imprimirLista("historialporordendeejecucion.txt", historial);

    
    // Aquí continúa la ejecución después de que todos los hilos han terminado

   
    

    

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

    for (int i = 0; i < 5; i++) {
        sem_init(&sem_prioridades[i], 0, 0);
    }
}