#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>


#define PROBABILIDAD_ENTRADA 0.5
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01
#define NODOSVECINOS 2



typedef enum {
    NO_INTERESADO, //0  El proceso no está interesado en acceder a la sección crítica actualmente
    SOLICITANTE, //1  El proceso está solicitando acceso a la sección crítica
    ESPERANDO   //2  El proceso está esperando su turno para acceder a la sección crítica
} Status;
typedef enum {
    SOLICITUD, //0 
    ACK,
    NACK
} Instruccion;

typedef struct {
    int id_nodo;        //Origen del paquete
    int id_proceso;     //Origen peticion PID
    int num_ticket;    //El número de ticket  
    Instruccion instruccion;
    Status estado;      //En que estado se encuentra
} Paquete;

Paquete creaPaquete(){
        // Crea un paquete
    Paquete paquete;
    paquete.id_nodo = 1;
    paquete.id_proceso = 123;
    paquete.num_ticket = 456;
    paquete.estado = SOLICITANTE;
    return paquete;
}

// Función dummy para enviar un mensaje a otro proceso
void NetworkSend(int destination, Paquete* message) {
    // TODO: Implementar la lógica de envío de mensaje a otro proceso
        // Enviamos el paquete al buzón correspondiente
    if (msgsnd(destination, message, sizeof(Paquete) - sizeof(long), 0) == -1) {
        perror("Error al enviar el mensaje");
        exit(1);
    }

    printf("PAQUETE ENVIADO A NODO %i.\n",message->id_nodo);
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

// Inicializa la cola de mensajes
int init_buzon(int IDNodo) {
    int msgid;
    // Creamos o accedemos a la cola de mensajes
    // TODO: Cambiar num de generar key por id del nodo
    key_t key=ftok("/bin/ls",IDNodo);
    if ((msgid = msgget(key, 0777 | IPC_CREAT)) == -1) {
        perror("Error al crear la cola de mensajes");
        exit(1);
    }
    return msgid;
}

//Memoria y variables del proceso
int identificador_nodo = 0;
sem_t esperaRespuesta;
Status estado = 0;
int lastticket=10; // Este es el mayor número de ticket recibido.


void* recepcion(void* args){

    //Tenemos que definir los tipos de cada uno de los paquetes.
    // Espera a recibir un mensaje en la cola de mensajes
    while(1){

        Paquete* recibido = networkrcv(identificador_nodo);
        if(recibido->instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO


        }else if(recibido->instruccion==ACK){
            //Nos están dando permiso para entrar SC
        }else if (recibido->instruccion==NACK){
            //No nos están dando el permiso para SC??
        }

    }

}

int main(int argc, char *argv[]) {
    int proj_id=52;
    int nodos[NODOSVECINOS-1];


    pid_t pid = getpid();

    //Creación del buzón del nodo
    key_t key=ftok("/bin/ls",proj_id);
    nodos[0]=msgget(key, IPC_CREAT | 0777);
    printf("Este nodo tiene IDENTIFICADOR: %i\n",nodos[0]);
	printf("Buzon id: %d creado", init_buzon(identificador_nodo));

    //Obtener las direcciones de los demás nodos
    for (int i=1; i<NODOSVECINOS; i++) {
        printf("Introduzca la ID del nodo %i",i);
        scanf("%i",&nodos[i]);
    }

    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);   


 do{
    // Tendríamos que hacer una función de inicialización de los buzones
    // Es necesario meter semaforos sobre la variable comptartida entre receptor y main (estado y ticket)
    // Contienda entre los procesos del mismo nodo. No sabes a que proceso envia. ESO SERIA FACIL SI SE ENVIA EL PROCESO EN EL MENSAJE ID PROCESO EN LAS PETICIONES Y EN LAS RESPUESTAS. Asi sabe para quien es.
    //UN PROCESO NO SABE SI LOS QUE QUIEREN ENTRAR ESTAN EN SU MISMO NODO O NO
    // EL RECEPTOR CONTESTA A LOS MENSAJES
    // COMUNICAR RECEPTOR CON PROCESOS EN EL MISMO NODO?
    // 


    
    //Aleatorizar la entrada en SC

        bool   quiero_entrar;
        
        do {
            quiero_entrar = (double)rand() / RAND_MAX > PROBABILIDAD_ENTRADA;
            if (!quiero_entrar) {
                // Si no quiero entrar espero
                sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
            }else{}
        }while (!quiero_entrar);

        //PASOS NECESARIOS PARA ENTRAR EN LA SC
        // 1 NOTIFICAR
        // 2 ESPERAR RESPUESTA
        // 3 EVALUAR RESPUESTA
        // SI OK: ENTRO -> Esta espera al OK se puede hacer con un semaforo

        //Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
        //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
        int ticketnum= lastticket + 1;
        
        //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

        for (int i=1;i<NODOSVECINOS; i++) {
            Paquete peticion;
            peticion.estado=SOLICITANTE;
            peticion.instruccion=SOLICITUD;
            peticion.id_nodo=nodos[0];
            peticion.num_ticket=ticketnum;
            peticion.id_proceso=pid;//Para futuro para poder direccionar en multiples procesos
            NetworkSend(nodos[i], &peticion); //TODO: Imprimir algun dato más desde esta función
        }

        //Ahora mismo todo el trabajo por parte del proceso está finalizado, se han enviado las peticiones y se encarga el receptor
        //La sincronización con el proceso se hace mediante un semaforo
        sem_wait(&esperaRespuesta);
        printf("[Nodo %i]: He entrado en la sección crítica",identificador_nodo);
        sleep(rand() % 5 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
        printf("[Nodo %i]: He terminado la sección crítica",identificador_nodo);



        //Salgo de la sección critica:
            //Cambio mi estado a que no quiero entrar
            quiero_entrar = false;
            // Cambio mi estado a que ya no hay limitacion
            estado=0;
            //Notifico a los nodos que ya salí de SC
            //De momento está en pseudocodigo
            for(i=0;i<cantidaddenodos;i++){
                //Enviar a cada nodo un mensaje de que ya salió de SC
                printf("[Nodo %i]: Mensaje enviado a nodo %i. Contenido: Ya no quiero entrar en SC\n",identificador_nodo,nodovecino);
            }

    }while (1); // Bucle para que funcione constantemente*/ 
    return 0;
}