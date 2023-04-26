#include <stdio.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>

typedef enum {
    NO_INTERESADO, //0  El proceso no está interesado en acceder a la sección crítica actualmente
    SOLICITANTE, //1  El proceso está solicitando acceso a la sección crítica
    ESPERANDO   //2  El proceso está esperando su turno para acceder a la sección crítica
} Status;

typedef struct {
    int id_nodo;        //Origen del paquete
    int id_proceso;     //Origen peticion PID
    int num_ticket;    //El número de ticket  
    Status estado;      //En que estado se encuentra
} Paquete;

// Inicializa la cola de mensajes
int init_buzon(int numNodo) {
    int msgid;
    // Creamos o accedemos a la cola de mensajes
    // TODO: Cambiar num de generar key por id del nodo
    key_t key=ftok("/bin/ls",numNodo);
    if ((msgid = msgget(key, 0777 | IPC_CREAT)) == -1) {
        perror("Error al crear la cola de mensajes");
        exit(1);
    }
    return msgid;
}

// Función dummy para recibir un mensaje de otro proceso
Paquete* networkrcv(int buzonRcv) {
    // TODO: Implementar la lógica de recepción de mensajes de otros procesos
    Paquete paquete; // Aquí se debe almacenar el mensaje recibido
    if (msgrcv(buzonRcv, &paquete, sizeof(Paquete) - sizeof(long), 0, 0) == -1) {
        perror("Error al recibir el mensaje");
        exit(1);
    }
    // Imprime el paquete recibido
    printf("Paquete recibido:\n");
    printf("  id_nodo: %d\n", paquete.id_nodo);
    printf("  id_proceso: %d\n", paquete.id_proceso);
    printf("  num_ticket: %d\n", paquete.num_ticket);
    printf("  estado: %d\n", paquete.estado);

    Paquete* paqueteRecibido = &paquete;

    return paqueteRecibido;
}

int main(int argc, char *argv[]) {


    int NumNodo = atoi(argv[1]);

    int msgid = init_buzon(NumNodo);

	printf("Buzon id: %d creado", msgid);

    // Espera a recibir un mensaje en la cola de mensajes
    while(1){
    networkrcv(msgid);
    }

    return 0;
}