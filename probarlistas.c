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



int main(int argc, char *argv[]) {
    /*
    //Signal de salida
    //printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);

    red=msgget(99999, IPC_CREAT | 0777);
    //printf("red establecida: %i\n",red);

    initparam(argc, argv);

    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);   
    pthread_t pthtest[10];
    for (int i =0; i<10; i++) {
        //printf("Creo hilo");
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,NULL);   
    }
   
    while (1) {
    
    }*/
    probarlista();
    return 0;
}



int probarlista() {
    struct Proceso* listaAleatoria = generarListaAleatoria(60);   // Generar una lista aleatoria de 10 elementos
    imprimirLista("listasinordenar.txt",listaAleatoria);
    ordenarCola(&listaAleatoria);
    imprimirLista("listaordenada.txt",listaAleatoria);    
    borrarLista(&listaAleatoria);   // Liberar la memoria de la lista
    return 0;
}
