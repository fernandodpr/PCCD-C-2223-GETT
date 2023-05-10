#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "datatypes.h"
#include "linkedlist.h"

void agregarProceso(struct Proceso** cabeza, struct Proceso* proceso) {
    struct Proceso* nuevoProceso = (struct Proceso*) malloc(sizeof(struct Proceso));
    memcpy(nuevoProceso, proceso, sizeof(struct Proceso)); // Hacer una copia del struct

    nuevoProceso->siguiente = NULL; // El siguiente del último elemento siempre es NULL
    if (*cabeza == NULL) { // Si la lista está vacía, el nuevo proceso es la cabeza
        *cabeza = nuevoProceso;
    } else {
        struct Proceso* ultimo = *cabeza;
        while (ultimo->siguiente != NULL) { // Recorrer la lista hasta encontrar el último elemento
            ultimo = ultimo->siguiente;
        }
        ultimo->siguiente = nuevoProceso; // Agregar el nuevo proceso después del último
    }
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
        printf("contador");
        contador++;

        procesoActual = procesoActual->siguiente;
    }

    if(procesoActual == NULL){
        printf("YA esta vacía\n");
    }
    
    return contador;
}

void ordenarCola(struct Proceso** cabeza) {
    struct Proceso* actual = *cabeza;
    struct Proceso* sigue = NULL;

    while(actual != NULL){
        struct Proceso* next = actual->siguiente;

        // Ordenar por prioridad (de menor a menor)
        if(sigue == NULL || actual->prioridad < sigue->prioridad){
            actual->siguiente = sigue;
            sigue = actual;
        
        // Ordenar por nodo (de menor a menor)
        }else if(actual->prioridad == sigue->prioridad && actual->idNodo > sigue->idNodo){
            actual->siguiente = sigue;
            sigue = actual;
        
        // Ordenar por ticket (de menor a menor)
        }else if(actual->prioridad == sigue->prioridad && actual->idNodo == sigue->idNodo && actual->ticket < sigue->ticket){
            actual->siguiente = sigue;
            sigue = actual;
        
        }else {
            struct Proceso *temp = sigue;
            while(temp->siguiente != NULL && temp->siguiente->prioridad < actual->prioridad){
                temp = temp->siguiente;
            }

            while(temp->siguiente != NULL && temp->siguiente->ticket < actual->ticket){
                temp = temp->siguiente;
            }

            while(temp->siguiente != NULL && temp->siguiente->idNodo < actual->idNodo){
                temp = temp->siguiente;
            }

            actual->siguiente = temp->siguiente;
            temp->siguiente = actual;
        }

        actual = next;

    }

    *cabeza = sigue;

 /*  struct Proceso* actual = *cabeza;
    struct Proceso* siguiente = NULL;
    int temp;
    bool cambio = true;

    printf("Acabo de entrar en ordenarCola\n");
   
    while (cambio) {
        cambio = false;
        actual = *cabeza;

        while (actual->siguiente != NULL) {
            
            siguiente = actual->siguiente;

            // Ordenar por prioridad (de menor a menor)
            if (actual->prioridad > siguiente->prioridad) {
                printf("Prioridad ordenada de mayor a menor\n");
                //actual->siguiente = siguiente->siguiente;
                //siguiente->siguiente = actual;
                //actual = siguiente;

                temp = actual->prioridad;
                actual->prioridad = siguiente->prioridad;
                siguiente->prioridad = temp;
                cambio = true;
            }
            // Si la prioridad es igual, ordenar por nodo (de menor a mayor)
            else if (actual->prioridad == siguiente->prioridad && actual->idNodo > siguiente->idNodo) {
                printf("Nodo ordenado de menor a mayor\n");
                temp = actual->idNodo;
                actual->idNodo = siguiente->idNodo;
                siguiente->idNodo = temp;
                cambio = true;
            }
            // Si la prioridad y el nodo son iguales, ordenar por tamaño de grupo (de mayor a menor)
            else if (actual->prioridad == siguiente->prioridad && actual->idNodo == siguiente->idNodo && actual->ticket > siguiente->ticket) {
                printf("ticket ordenado de mayor a menor\n");
                temp = actual->ticket;
                actual->ticket = siguiente->ticket;
                siguiente->ticket = temp;
                cambio = true;
            }
            actual = actual->siguiente;
        }

    }*/
}

void imprimirLista(char* rutaArchivo, struct Proceso* cabeza) {
    FILE* archivo = fopen(rutaArchivo, "w");
    
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        return;
    }
    
    struct Proceso* procesoActual = cabeza;
    fprintf(archivo, "ID Proceso,Prioridad,ID Nodo,Ticket,Hora de creación del proceso,Hora de entrada a la SC,Hora de salida de la SC,Hora de muerte del proceso,ACK's\n");
    
    while (procesoActual != NULL) {
        struct tm *tm_info = localtime(&procesoActual->creado);
        char tiempo_inicio[20];
        strftime(tiempo_inicio, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%d,%d,%d,%d,%s.%01ld,", procesoActual->idProceso, procesoActual->prioridad, procesoActual->idNodo, procesoActual->ticket, tiempo_inicio, procesoActual->inicio % 1000);
        
        tm_info = localtime(&procesoActual->inicio);
        char tiempo_creado[20];
        strftime(tiempo_creado, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%01ld,", tiempo_creado, procesoActual->creado % 1000);
        
        tm_info = localtime(&procesoActual->atendido);
        char tiempo_atendido[20];
        strftime(tiempo_atendido, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%01ld,", tiempo_atendido, procesoActual->atendido % 1000);
        
        tm_info = localtime(&procesoActual->fin);
        char tiempo_fin[20];
        strftime(tiempo_fin, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%01ld,%d\n", tiempo_fin, procesoActual->fin % 1000, procesoActual->contACK);
        
        procesoActual = procesoActual->siguiente;
    }
    
    fclose(archivo);
}

struct Proceso* generarListaAleatoria(int cantidad) {
    struct Proceso* cabeza = NULL;
    srand(time(NULL));  // Inicializar el generador de números aleatorios
    for (int i = 0; i < cantidad; i++) {
        struct Proceso* nuevoProceso = (struct Proceso*) malloc(sizeof(struct Proceso));
        nuevoProceso->idProceso = rand() % 1000;   // ID de proceso aleatorio entre 0 y 999
        nuevoProceso->prioridad = rand() % 4;   // Prioridad aleatoria entre 0 y 3
        nuevoProceso->idNodo = rand() % 10;   // ID de nodo aleatorio entre 0 y 9
        nuevoProceso->ticket = rand() % 10000;   // Ticket aleatorio entre 0 y 9999
        nuevoProceso->inicio = time(NULL) - rand() % 3600;   // Hora de creación aleatoria en la última hora
        nuevoProceso->creado = nuevoProceso->inicio + rand() % 60;   // Hora de entrada a la SC aleatoria en la última hora
        nuevoProceso->atendido = nuevoProceso->creado + rand() % 60;   // Hora de salida de la SC aleatoria en la última hora
        nuevoProceso->fin = nuevoProceso->atendido + rand() % 3600;   // Hora de muerte aleatoria en la siguiente hora
        nuevoProceso->retardo = rand() % 10;   // Retardo aleatorio entre 0 y 9     
        nuevoProceso->siguiente = cabeza;
        cabeza = nuevoProceso;
    }
    
    return cabeza;
}
bool esIgual(struct Proceso* cabeza, struct Proceso* proceso) {
    if(cabeza->idProceso != proceso->idProceso) {
        printf("NOEsIgual: idProceso\n");
        return false;
    }
    if(cabeza->prioridad != proceso->prioridad) {
        printf("NOEsIgual: prioridad\n");
                        printf("La prioridad de cabeza es %i    \n",cabeza->prioridad);
                printf("La prioridad de proceso es %i    \n",proceso->prioridad);

        return false;
    }
    if(cabeza->idNodo != proceso->idNodo) {
                printf("NOEsIgual: idNodo\n");



        return false;
    }
    if(cabeza->ticket != proceso->ticket) {
                printf("NOEsIgual: ticket\n");

        return false;
    }
    if(cabeza->inicio != proceso->inicio) {
                printf("NOEsIgual: inicio\n");

        return false;
    }
    if(cabeza->creado != proceso->creado) {
                printf("NOEsIgual: creado\n");

        return false;
    }
    if(cabeza->atendido != proceso->atendido) {
                printf("NOEsIgual: atendido\n");

        return false;
    }
    if(cabeza->fin != proceso->fin) {
                printf("NOEsIgual: fin\n");

        return false;
    }
    if(cabeza->retardo != proceso->retardo) {
                printf("NOEsIgual: retardo\n");

        return false;
    }
    if(cabeza->contACK != proceso->contACK) {
                printf("NOEsIgual: contACK\n");

        return false;
    }
    if(cabeza->ejecucion != proceso->ejecucion) {
                printf("NOEsIgual: ejecucion\n");

        return false;
    }
    if(cabeza->pedirPermiso != proceso->pedirPermiso) {
                printf("NOEsIgual: pedirpermiso\n");

        return false;
    }
    return true;
}

void eliminarCabeza(struct Proceso** cabeza) {
    if (*cabeza == NULL) {
        return; // La lista está vacía
    }
    struct Proceso* siguiente = (*cabeza)->siguiente;
    if (siguiente == NULL) {
        *cabeza = NULL; // La lista tiene un solo elemento
    } else {
        *cabeza = siguiente; // La lista tiene más de un elemento
    }
}

bool compararIdNodo(struct Proceso* cabeza, int idNodoBuscado) {
    if (cabeza == NULL) { // Si la cola está vacía, retorna false
        return false;
    }
    else { // Si la cola tiene al menos un elemento
        if (cabeza->idNodo == idNodoBuscado) { // Si el idNodo de la cabeza es igual al idNodo buscado, retorna true
            return true;
        }
        else { // Si el idNodo de la cabeza no es igual al idNodo buscado, retorna false
            return false;
        }
    }
}

int ACKproceso(struct Proceso* lista, int idProceso){
    struct Proceso* actual = lista;
    while (actual != NULL) {
        if (actual->idProceso == idProceso) {
            return actual->contACK;
        }
        actual = actual->siguiente;
    }

    return -1;

}

bool procesoSC(struct Proceso* cabeza) {
    //bool respuesta = 0;
    struct Proceso* procesoActual = cabeza;
    
    while (procesoActual != NULL) {
        if(procesoActual->ejecucion==1){
            return true;
        }
        procesoActual = procesoActual->siguiente;
    }

    if(procesoActual == NULL){
        printf("YA esta vacía sc\n");
    }
    
    return false;
}
void addACK(struct Proceso* lista, int idProceso) {
    struct Proceso* actual = lista;
    while (actual != NULL) {
        if (actual->idProceso == idProceso) {
            actual->contACK++;
            printf("El proceso %i tiene %i ACK's",idProceso,actual->contACK);
            break;
        }
        actual = actual->siguiente;
    }
}

void necesariapeticion(struct Proceso* cabeza,int minodo){
    struct Proceso* procesoActual = cabeza;
    while (procesoActual != NULL) {
        if(procesoActual->idNodo!=minodo){
            if(procesoActual->siguiente->idNodo==minodo){
                procesoActual->siguiente->pedirPermiso=true;
            }
            return;
        }
        procesoActual = procesoActual->siguiente;
    }
    return;
}