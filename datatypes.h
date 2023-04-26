#ifndef DATATYPES_H
#define DATATYPES_H
    typedef enum {
        NO_INTERESADO, //0  El proceso no está interesado en acceder a la sección crítica actualmente
        SOLICITANTE, //1  El proceso está solicitando acceso a la sección crítica
        FINALIZADO   //2  El proceso está esperando su turno para acceder a la sección crítica
    } Status;

    typedef enum {
        SOLICITUD, //0 
        ACK,
        NACK
    } Instruccion;


#endif

