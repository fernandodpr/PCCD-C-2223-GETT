#include "datatypes.h"

#ifndef NETWORK_H
#define NETWORK_H
    typedef struct {
        long mtype;          //Destinatario DEST
        int id_nodo;        //Remitente SRC
        int id_proceso;     //Origen peticion PID
        int num_ticket;     //El número de ticket  
        Instruccion instruccion;
        Status estado;      //En que estado se encuentra
    } Paquete;

    Paquete* networkrcv(int red,int nodo);
    void NetworkSend(int red,int nodo, int destinatario,int estado, int pid,int instruccion,int ticket);
#endif