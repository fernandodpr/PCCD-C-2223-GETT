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

    // Hacer una copia del paquete y devolver un puntero a la misma
    Paquete* paqueteRecibido = malloc(sizeof(Paquete));
    memcpy(paqueteRecibido, &paquete, sizeof(Paquete));

    return paqueteRecibido;
}
void NetworkSend(int red, int destinatario,int estado,int instruccion,struct Proceso processinc) {
    char* estadostring[3] = { "NO_INTERESADO", "SOLICITANTE", "FINALIZADO"};
    char* tipostring[3] = { "SOLICITUD", "ACK", "NACK"};        
    //TODO: Imprimir algun dato más desde esta función Como un resumen del paquete
    // TODO: Implementar la lógica de envío de mensaje a otro proceso
    // Enviamos el paquete al buzón correspondiente
    Paquete message;
    message.estado=estado;
    message.process=processinc;
    message.instruccion=instruccion;
    message.mtype=destinatario;
    
    //printf("Soy la función Send y me han pasado el parámetro RED: %i\n", red);
    
    //Simulacion de retardo de red Hemos elegido valores aleatorios de entre 1ms y 10ms    
    int tiempo_dormido = rand() % 9000 + 1000; // generar un número aleatorio entre 1000 y 10000
    usleep(tiempo_dormido); // dormir el programa durante el tiempo generado



    if (msgsnd(red, (struct msgbuf *)&message, sizeof(Paquete), 0) == -1) {
       printf("ERROR NO ENVIADO: Red:%i  Destino: %li   Instruccion: %i\n", red, message.mtype, message.instruccion);
        exit(1);
    }else{
        //printf("[Nodo %i][ENVIADO] Envío %s a nodo %i. Numero de ticket %i \n",nodo,tipostring[instruccion],destinatario,ticket);
    }


}