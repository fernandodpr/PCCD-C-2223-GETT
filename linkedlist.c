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
    *cabeza = nuevoProceso;
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
void ordenarCola(struct Proceso** cabeza) {
    struct Proceso* actual = *cabeza;
    struct Proceso* siguiente = NULL;
    int temp;
    bool cambio = true;
    
    while (cambio) {
        cambio = false;
        actual = *cabeza;
        while (actual->siguiente != NULL) {
            siguiente = actual->siguiente;
            // Ordenar por prioridad (de mayor a menor)
            if (actual->prioridad < siguiente->prioridad) {
                temp = actual->prioridad;
                actual->prioridad = siguiente->prioridad;
                siguiente->prioridad = temp;
                cambio = true;
            }
            // Si la prioridad es igual, ordenar por nodo (de menor a mayor)
            else if (actual->prioridad == siguiente->prioridad && actual->idNodo > siguiente->idNodo) {
                temp = actual->idNodo;
                actual->idNodo = siguiente->idNodo;
                siguiente->idNodo = temp;
                cambio = true;
            }
            // Si la prioridad y el nodo son iguales, ordenar por tamaño de grupo (de mayor a menor)
            else if (actual->prioridad == siguiente->prioridad && actual->idNodo == siguiente->idNodo && actual->idProceso < siguiente->idProceso) {
                temp = actual->idProceso;
                actual->idProceso = siguiente->idProceso;
                siguiente->idProceso = temp;
                cambio = true;
            }
            actual = actual->siguiente;
        }
    }
}
void imprimirLista(char* rutaArchivo, struct Proceso* cabeza) {
    FILE* archivo = fopen(rutaArchivo, "w");
    
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        return;
    }
    
    struct Proceso* procesoActual = cabeza;
    
    while (procesoActual != NULL) {
        fprintf(archivo, "Valor: %d\n", procesoActual->valor);
        fprintf(archivo, "ID Proceso: %d\n", procesoActual->idProceso);
        fprintf(archivo, "Prioridad: %d\n", procesoActual->prioridad);
        fprintf(archivo, "ID Nodo: %d\n", procesoActual->idNodo);
        fprintf(archivo, "Ticket: %d\n", procesoActual->ticket);
        fprintf(archivo, "Hora de creación del proceso: %ld\n", procesoActual->inicio);
        fprintf(archivo, "Hora de entrada a la SC: %ld\n", procesoActual->creado);
        fprintf(archivo, "Hora de salida de la SC: %ld\n", procesoActual->atendido);
        fprintf(archivo, "Hora de muerte del proceso: %ld\n", procesoActual->fin);
        fprintf(archivo, "Retardo: %d\n", procesoActual->retardo);
        procesoActual = procesoActual->siguiente;
    }
    
    fclose(archivo);
}

