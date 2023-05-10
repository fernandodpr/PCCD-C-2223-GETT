#ifndef TIMER
#define TIMER
#include <time.h>
#endif


#ifndef LINKEDLIST_H
#define LINKEDLIST_H
    #include "datatypes.h"
    struct Proceso {
        int idProceso;         // Id del proceso
        int prioridad;          // Prioridad (Anulaciones, Pagos, Admin y Reservas, Consultas)
        int idNodo;             // Id del nodo del 
        int ticket;
        time_t inicio;          // Hora de inicio SC
        time_t creado;          //Hora de creación del proceso
        time_t atendido;        // Hora de salida de la SC
        time_t fin;             // Hora en el que se mata al proceso
        int retardo;
        int contACK;
        int ejecucion;          // Valor booleano
        int pedirPermiso;       // Otro valor booleano       
        long mtype;          //Destinatario DEST
        int instruccion;
        int estado;
        
        struct Proceso* siguiente;
    };
#endif
    //FUNCIONES

    void borrarLista(struct Proceso** cabeza);
    void agregarProceso(struct Proceso** cabeza, struct Proceso* proceso);
    void ordenarCola(struct Proceso** cabeza);
    int contarProcesos(struct Proceso* cabeza);
    void necesariapeticion(struct Proceso* cabeza,int minodo);
    void imprimirLista(char* rutaArchivo, struct Proceso* cabeza);
    struct Proceso* generarListaAleatoria(int cantidad);
    bool esIgual(struct Proceso* cabeza, struct Proceso* proceso);
    void eliminarCabeza(struct Proceso** cabeza);

    bool compararIdNodo(struct Proceso* cabeza, int idNodoBuscado);

    void addACK(struct Proceso* lista, int idProceso);
    bool procesoSC(struct Proceso* cabeza);
    int ACKproceso(struct Proceso* lista, int idProceso);
