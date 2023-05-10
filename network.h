#include "linkedlist.h"

#ifndef NETWORK_H
#define NETWORK_H
    typedef struct {
        long mtype;          //Destinatario DEST
        int idNodo;
        int ticket;
        int prioridad;
        int instruccion;
        int estado;
        int proceso;
    } Paquete;

#endif
    void networkrcv(int red,int nodo,Paquete* message);
    
    void NetworkSend(int red ,struct Proceso* proceso,Paquete* paquete);