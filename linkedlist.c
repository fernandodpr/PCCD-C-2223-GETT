#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "network.h"
#include "datatypes.h"
#include "linkedlist.h"


void agregarProceso(struct Proceso** cabeza, int valor) {
    struct Proceso* nuevoProceso = (struct Proceso*) malloc(sizeof(struct Proceso));
    nuevoProceso->valor = valor;
    nuevoProceso->siguiente = *cabeza;
    *cabeza = nuevoNodo;
}
void borrarLista(struct Proceso** cabeza) {
    struct Proceso* procesoActual = *cabeza;
    struct Proceso* procesoSiguiente = NULL;
    
    while (procesoActual != NULL) {
        procesoSiguiente = procesoActual->siguiente;
        free(procesoActual);
        procesoActual = procesoSiguiente;
    }
    
    *cabeza = NULL;
}
int contarProcesos(struct Proceso* cabeza) {
    int contador = 0;
    struct Proceso* procesoActual = cabeza;
    
    while (procesoActual != NULL) {
        contador++;
        procesoActual = procesoActual->siguiente;
    }
    
    return contador;
}