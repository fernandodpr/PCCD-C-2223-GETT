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


void agregarProceso(struct Proceso** cabeza, int prioridad, int ticket, int nodo) {
    struct Proceso* nuevoProceso = (struct Proceso*) malloc(sizeof(struct Proceso));
    nuevoProceso->prioridad = prioridad;
    nuevoProceso->ticket = ticket;
    nuevoProceso->idNodo = nodo;

    nuevoProceso->siguiente = *cabeza;
    *cabeza = nuevoProceso;
    printf("Prioridad: %d, Ticket: %d, idNodo: %d\n", (*cabeza)->prioridad, (*cabeza)->ticket, (*cabeza)->idNodo);
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
    printf("Hola k haces");
    struct Proceso* actual = *cabeza;
    struct Proceso* siguiente = NULL;
    int temp;
    bool cambio = true;

    printf("Acabo de entrar en ordenarCola\n");
   
    while (cambio) {
        cambio = false;
        actual = *cabeza;

        printf("Dentro del bucle 1\n");
         
        while (actual->siguiente != NULL) {
            siguiente = actual->siguiente;
            
            printf("Dentro del bucle 2\n");
            
            printf("%d < %d es %d  (Prioridad actual < Prioridad siguiente)\n", actual->prioridad, siguiente->prioridad, actual->prioridad < siguiente->prioridad);
            printf("%d > %d es %d  (Nodo actual > Nodo siguiente)\n", actual->idNodo, siguiente->idNodo, actual->idNodo > siguiente->idNodo);
            printf("%d < %d es %d  (Ticket actual < Ticket siguiente)\n", actual->ticket, siguiente->ticket, actual->ticket < siguiente->ticket);
            // Ordenar por prioridad (de mayor a menor)
            if (actual->prioridad < siguiente->prioridad) {
                printf("Prioridad ordenada de mayor a menor\n");
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
            else if (actual->prioridad == siguiente->prioridad && actual->idNodo == siguiente->idNodo && actual->ticket < siguiente->ticket) {
                printf("ticket ordenado de mayor a menor\n");
                temp = actual->ticket;
                actual->ticket = siguiente->ticket;
                siguiente->ticket = temp;
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
        fprintf(archivo, "ID Proceso,Prioridad,ID Nodo,Ticket,Hora de creación del proceso,Hora de entrada a la SC,Hora de salida de la SC,Hora de muerte del proceso,Retardo\n");
    
    while (procesoActual != NULL) {
        struct tm *tm_info = localtime(&procesoActual->inicio);
        char tiempo_inicio[20];
        strftime(tiempo_inicio, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%d,%d,%d,%d,%s.%03ld,", procesoActual->idProceso, procesoActual->prioridad, procesoActual->idNodo, procesoActual->ticket, tiempo_inicio, procesoActual->inicio % 1000);
        
        tm_info = localtime(&procesoActual->creado);
        char tiempo_creado[20];
        strftime(tiempo_creado, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%03ld,", tiempo_creado, procesoActual->creado % 1000);
        
        tm_info = localtime(&procesoActual->atendido);
        char tiempo_atendido[20];
        strftime(tiempo_atendido, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%03ld,", tiempo_atendido, procesoActual->atendido % 1000);
        
        tm_info = localtime(&procesoActual->fin);
        char tiempo_fin[20];
        strftime(tiempo_fin, 20, "%H:%M:%S", tm_info);
        fprintf(archivo, "%s.%03ld,%d\n", tiempo_fin, procesoActual->fin % 1000, procesoActual->retardo);
        
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
        return false;
    }
    if(cabeza->prioridad != proceso->prioridad) {
        return false;
    }
    if(cabeza->idNodo != proceso->idNodo) {
        return false;
    }
    if(cabeza->ticket != proceso->ticket) {
        return false;
    }
    if(cabeza->inicio != proceso->inicio) {
        return false;
    }
    if(cabeza->creado != proceso->creado) {
        return false;
    }
    if(cabeza->atendido != proceso->atendido) {
        return false;
    }
    if(cabeza->fin != proceso->fin) {
        return false;
    }
    if(cabeza->retardo != proceso->retardo) {
        return false;
    }
    if(cabeza->contACK != proceso->contACK) {
        return false;
    }
    if(cabeza->ejecucion != proceso->ejecucion) {
        return false;
    }
    if(cabeza->pedirPermiso != proceso->pedirPermiso) {
        return false;
    }
    return true;
}


void eliminarCabeza(struct Proceso** cabeza) {
    if (*cabeza == NULL || (*cabeza)->siguiente == NULL) {
        return; // La lista está vacía o solo tiene un elemento
    }
    struct Proceso* segundo = (*cabeza)->siguiente;
    *cabeza = segundo; // Cambia la cabeza por el segundo elemento
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
