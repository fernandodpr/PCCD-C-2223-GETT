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



Paquete* networkrcv(int red,int nodo) {
    Paquete paquete;
    //printf("Esperando recibir un paquete por red %i para mi (%i)",red,nodos[0]);
    fflush(stdout);
    if (msgrcv(red, (struct msgbuf *)&paquete, sizeof(Paquete),(long)nodo, 0) == -1) {
        perror("Error al recibir el mensaje");
        exit(1);
    }

    // Imprime el paquete recibido
    //printf("Paquete recibido:  ");
    //printf("  id_nodo: %d  ", paquete.id_nodo);
    //printf("  destinatario mtype: %ld  ", paquete.mtype);
    //printf("  id_proceso: %d  ", paquete.id_proceso);
    //printf("  num_ticket: %d  ", paquete.num_ticket);
    //printf("  estado: %d\n", paquete.estado);

    // Hacer una copia del paquete y devolver un puntero a la misma
    Paquete* paqueteRecibido = malloc(sizeof(Paquete));
    memcpy(paqueteRecibido, &paquete, sizeof(Paquete));

    return paqueteRecibido;
}
void NetworkSend(int red,int nodo, int destinatario,int estado, int pid,int instruccion,int ticket) {
    char* estadostring[3] = { "NO_INTERESADO", "SOLICITANTE", "FINALIZADO"};
    char* tipostring[3] = { "SOLICITUD", "ACK", "NACK"};        
    //TODO: Imprimir algun dato más desde esta función Como un resumen del paquete
    // TODO: Implementar la lógica de envío de mensaje a otro proceso
    // Enviamos el paquete al buzón correspondiente
    Paquete message;
    message.estado=estado;
    message.id_nodo=nodo;
    message.id_proceso=pid;
    message.instruccion=instruccion;
    message.mtype=destinatario;
    message.num_ticket=ticket;
    
    //printf("Soy la función Send y me han pasado el parámetro RED: %i\n", red);
    
    if (msgsnd(red, (struct msgbuf *)&message, sizeof(Paquete), 0) == -1) {
       printf("ERROR NO ENVIADO: Red:%i  Destino: %li   Instruccion: %i\n", red, message.mtype, message.instruccion);
        exit(1);
    }else{
        printf("[Nodo %i][ENVIADO] Envío %s a nodo %i. Numero de ticket %i \n",nodo,tipostring[instruccion],destinatario,ticket);
    }


}