#ifndef TIMER
#define TIMER
#include <time.h>
#endif


#ifndef LINKEDLIST_H
#define LINKEDLIST_H


    struct Proceso {
       int idProceso;         // Id del proceso
        int prioridad;          // Prioridad (Anulaciones, Pagos, Admin y Reservas, Consultas)
        int idNodo;             // Id del nodo del 
        int ticket;
        time_t inicio;          // Hora de creacion del proceso
        time_t creado;       
        time_t atendido;        // Hora de salida de la SC
        time_t fin;             // Hora en el que se mata al proceso
        int retardo;
        int contACK;
        int ejecucion;          // Valor booleano
        int pedirPermiso;       // Otro valor booleano
        
        struct Proceso* siguiente;
    };

    //FUNCIONES

    void borrarLista(struct Proceso** cabeza);
    void agregarProceso(struct Proceso** cabeza, int valor);
    int contarProcesos(struct Proceso* cabeza);
#endif