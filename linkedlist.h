#ifndef TIMER
#define TIMER
#include <time.h>
#endif

#ifndef LINKEDLIST_H
#define LINKEDLIST_H


    struct Proceso {
        int idProceso;
        int prioridad;
        int idNodo;
        time_t inicio;
        time_t fin;
        int retardo;
        struct Proceso* siguiente;
    };

    //FUNCIONES

    void borrarLista(struct Proceso** cabeza);
    void agregarProceso(struct Proceso** cabeza, int valor);
    int contarProcesos(struct Proceso* cabeza);
#endif