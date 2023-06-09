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


void agregarNodo(struct Nodo** cabeza, int valor) {
    struct Nodo* nuevoNodo = (struct Nodo*) malloc(sizeof(struct Nodo));
    nuevoNodo->valor = valor;
    nuevoNodo->siguiente = *cabeza;
    *cabeza = nuevoNodo;
}
void borrarLista(struct Nodo** cabeza) {
    struct Nodo* nodoActual = *cabeza;
    struct Nodo* nodoSiguiente = NULL;
    
    while (nodoActual != NULL) {
        nodoSiguiente = nodoActual->siguiente;
        free(nodoActual);
        nodoActual = nodoSiguiente;
    }
    
    *cabeza = NULL;
}
int contarNodos(struct Nodo* cabeza) {
    int contador = 0;
    struct Nodo* nodoActual = cabeza;
    
    while (nodoActual != NULL) {
        contador++;
        nodoActual = nodoActual->siguiente;
    }
    
    return contador;
}