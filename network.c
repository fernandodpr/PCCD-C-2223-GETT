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



/*
void networkrcv(int red,int nodo,struct Proceso* proceso) {
    struct Proceso paquete;
    printf("Esperando recibir un paquete por red %i para mi (%i) de tamano %i\n",red,nodo,sizeof(struct Proceso));

    if (msgrcv(red, (struct msgbuf *)&paquete, sizeof(struct Proceso),0, 0) == -1) {
        perror("Error al recibir el mensaje");
    }
    memcpy(proceso, &paquete, sizeof(struct Proceso));
    printf("Recibido un paquete por red %i para mi (%i)",red,nodo);

    return;
}
void NetworkSend(int red, struct Proceso* proceso) {
    
    struct Proceso message;
    memcpy(&message, proceso, sizeof(struct Proceso));

    printf("Envio un paquete para: %i",proceso->mtype);
    if(msgsnd(red, &message, sizeof(struct Proceso), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}*/

    void networkrcv(int red,int nodo,Paquete* message){
        int resultado=msgrcv(red, (struct msgbuf *)message, sizeof(Paquete),(long)nodo, 0);
        
        if (resultado== -1) {
            printf("Error al recibir el mensaje");
            exit(-1);
        }

        return;
    }
    void NetworkSend(int red ,struct Proceso* proceso,Paquete* paquete){
        Paquete message;
    if(paquete==NULL){
        message.mtype=proceso->mtype;
        message.idNodo=proceso->idNodo;
        message.ticket=proceso->ticket;
        message.prioridad=proceso->prioridad;
        message.instruccion=proceso->instruccion;
        message.estado=proceso->estado;
        message.proceso=proceso->idProceso;
    }else{
        message=*paquete;
    }
        printf("[RED %i] Envio paquete a nodo %li con prioridad %i con instruccion %i\n",red,message.mtype,message.prioridad,message.instruccion);
        if(msgsnd(red, &message, sizeof(message), 0) == -1) {
          perror("msgsnd");
             exit(1);
        }
}