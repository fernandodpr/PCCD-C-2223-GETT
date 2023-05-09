#include "linkedlist.h"
#ifndef NETWORK_H
    #define NETWORK_H
    #include "datatypes.h"
    typedef struct {
        long mtype;          //Destinatario DEST
        Instruccion instruccion;
        Status estado;      //En que estado se encuentra
        struct Proceso process;
    } Paquete;

    Paquete* networkrcv(int red,int nodo);
    void NetworkSend(int red, int destinatario,int estado,int instruccion,struct Proceso processinc);
#endif