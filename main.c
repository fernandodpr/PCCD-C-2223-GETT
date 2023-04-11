#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

///////////////////////////////////
#define PROBABILIDAD_ENTRADA 0.05
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01
#define NODOSVECINOS 4



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

struct Nodo {
    int valor;
    struct Nodo* siguiente;
};

typedef struct {
    int id_nodo;        //Remitente SRC
    int mtype;          //Destinatario DEST


    int id_proceso;     //Origen peticion PID
  
    
    
    int num_ticket;     //El número de ticket  
    Instruccion instruccion;
    Status estado;      //En que estado se encuentra
} Paquete;

//Memoria y variables del proceso
int identificador_nodo = 0;
int red =0;
sem_t esperaRespuesta;
Status estado = 0;

int lastticket=78; // Este es el mayor número de ticket recibido.
int ticketnum;  // Este es el numero de ticket que yo estoy usando
struct Nodo* nodosenespera = NULL; //Lista con los nodos en espera

int nodos[NODOSVECINOS-1]; //IMPORTANTE en nodos[0] siempre está mi ID

/////////////////////////////////////////////////////////////////////////////////
Paquete creaPaquete(){
    // Crea un paquete
    Paquete paquete;
    paquete.id_nodo = 1;
    paquete.id_proceso = 123;
    paquete.num_ticket = 456;
    paquete.estado = SOLICITANTE;
    return paquete;
}

void agregarNodo(struct Nodo** cabeza, int valor) {
    struct Nodo* nuevoNodo = (struct Nodo*) malloc(sizeof(struct Nodo));
    nuevoNodo->valor = valor;
    nuevoNodo->siguiente = *cabeza;
    *cabeza = nuevoNodo;
}

// Función dummy para enviar un mensaje a otro proceso
void NetworkSend(int red,int destination, Paquete* message) {
    //TODO: Imprimir algun dato más desde esta función Como un resumen del paquete
    // TODO: Implementar la lógica de envío de mensaje a otro proceso
    // Enviamos el paquete al buzón correspondiente
    message->mtype=destination;
    printf("El destino: %i",message->mtype);

    if (msgsnd(red, (struct msgbuf *)&message, sizeof(Paquete) - sizeof(long), 0) == -1) {
        perror("Error al enviar el mensaje");
        exit(1);
    }



    printf("PAQUETE ENVIADO A NODO %i.\n",destination);
}
// Función dummy para recibir un mensaje de otro proceso
Paquete* networkrcv(int red) {
    // TODO: Implementar la lógica de recepción de mensajes de otros procesos
    Paquete paquete; // Aquí se debe almacenar el mensaje recibido
    printf("Esperando recibir un paquete por red %i para mi (%i)",red,nodos[0]);
    if (msgrcv(red, &paquete, sizeof(Paquete) - sizeof(long), nodos[0], 0) == -1) {
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



void* recepcion(void* args){

    //Tenemos que definir los tipos de cada uno de los paquetes.
    // Espera a recibir un mensaje en la cola de mensajes
    int acks = 0;
    while(1){
        printf("Escuchando...");
        Paquete* recibido = networkrcv(red);
        
        
        if(recibido->instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO
            //Primero es necesario conocer si hay contienda mediante mi estado
            if(estado==0){
                //Dejamos que pase el otro proceso:   MOTIVO --> //No estoy interesado
                if(recibido->num_ticket>lastticket) lastticket=recibido->num_ticket; // Si el ticket que recibo es mayor actualizo

                //Mando el ACK
                Paquete respuesta;
                respuesta.estado=NO_INTERESADO;
                respuesta.instruccion=ACK;
                respuesta.id_nodo=nodos[0];
                respuesta.num_ticket=0;//No es relevante en este caso
                respuesta.id_proceso=0;//No es relevante en este caso
                NetworkSend(red,recibido->id_nodo, &respuesta); 



    //En estos casos existe contienda.
            }else if (estado==1 && (recibido->num_ticket<ticketnum) ){ //Pasa el nodo con ticket menor, el otro
                //Dejamos que pase el otro proceso:   MOTIVO --> //Su ticket es menor
                //Mando el ACK
                Paquete respuesta;
                respuesta.estado=SOLICITANTE;
                respuesta.instruccion=ACK;
                respuesta.id_nodo=nodos[0];
                respuesta.num_ticket=0;//No es relevante en este caso
                respuesta.id_proceso=0;//No es relevante en este caso
                NetworkSend(red,recibido->id_nodo, &respuesta); 

            }else if(estado==1 && (recibido->num_ticket==ticketnum) ){ //Tenemos el mismo numero de ticket, resolvemos con el ID nodo
                if(recibido->id_nodo<nodos[0]){
                    //Entra el nodo con ID menor
                    Paquete respuesta;
                    respuesta.estado=SOLICITANTE;
                    respuesta.instruccion=ACK;
                    respuesta.id_nodo=nodos[0];
                    respuesta.num_ticket=0;//No es relevante en este caso
                    respuesta.id_proceso=0;//No es relevante en este caso
                    NetworkSend(red,recibido->id_nodo, &respuesta); 
                }else{
                    //No autoricé al nodo, tengo que despertarlo cuando termine
                    agregarNodo(&nodosenespera,recibido->id_nodo);
                }

            }else if(estado==1 && (recibido->num_ticket>ticketnum)){
                //Tengo un ticket menor al del solicitante asi que lo agrego a la lista
                agregarNodo(&nodosenespera,recibido->id_nodo);

            }
        }else if(recibido->instruccion==ACK){
            //Nos están dando permiso para entrar SC
            //Primera comprobación, realmente, queremos?
            if(estado!=0){ // Si mi estado no es 0 significa que quiero entrar
                //Cuando puedo entar a SC? Cuando tengo permiso de todos los nodos
                acks++; //Hemos recibido un ACK
                printf("Nuevo ACK recibido. %i",acks);
                if (acks==NODOSVECINOS-1){
                    //Tenemos los permisos necesarios para acceder a SC
                    //Aviso al proceso de que pase
                    sem_post(&esperaRespuesta);
                    printf("RECEPTOR: Se ha notificado al proceso de que tiene permisos para entar.");
                }
            }
        }else if (recibido->instruccion==NACK){
            //No nos están dando el permiso para SC??
        }

    }

}

void sigint_handler(int sig) {
    printf("Se ha presionado Ctrl+C eliminando buzones....\n");
    if (msgctl(red, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    printf("Buzón eliminado.\n");
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {

    printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);

    int proj_id=52; //TODO: Esto hay que parametrizarlo
    pid_t pid = getpid();

    //Creación del buzón del nodo
    key_t key=ftok("/bin/ls",proj_id);
    red=msgget(99999, IPC_CREAT | 0777);
    
    
    printf("Introduzca la ID del nodo");
    scanf("%i",&nodos[0]);


    printf("Este nodo tiene IDENTIFICADOR: %i\n",nodos[0]);
	printf("Buzon id: %d creado", init_buzon(identificador_nodo));

    //Obtener las direcciones de los demás nodos
    for (int i=1; i<NODOSVECINOS; i++) {
        fflush(stdin);
        printf("Introduzca la ID del nodo %i",i);
        scanf("%i",&nodos[i]);
        //nodos[i]=i;
    }
    for (int i=0; i<NODOSVECINOS; i++) {

        printf("ID de nodo %i:   %i",i,nodos[i]);

        //nodos[i]=i;
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
       
        do {
            estado = (double)rand() / RAND_MAX > PROBABILIDAD_ENTRADA;
            if (!estado) {
                // Si no quiero entrar espero
                sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
            }else{}
        }while (!estado); // 0 No interesado 1 SOLICITANTE

        //PASOS NECESARIOS PARA ENTRAR EN LA SC
        // 1 NOTIFICAR
        // 2 ESPERAR RESPUESTA
        // 3 EVALUAR RESPUESTA
        // SI OK: ENTRO -> Esta espera al OK se puede hacer con un semaforo

        //Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
        //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
        ticketnum= lastticket + rand() % 5 + 4;
        printf("Numero de ticket %i",ticketnum);
        
        //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

        for (int i=1;i<NODOSVECINOS; i++) {
            printf("Hola");
            Paquete peticion;
            peticion.estado=SOLICITANTE;
            peticion.instruccion=SOLICITUD;
            peticion.id_nodo=nodos[0];
            peticion.num_ticket=ticketnum;
            peticion.id_proceso=pid;//Para futuro para poder direccionar en multiples procesos
            printf("Nodo destino: %i\n",nodos[i]);
            NetworkSend(red,nodos[i], &peticion); //TODO: Imprimir algun dato más desde esta función
        }

        //Ahora mismo todo el trabajo por parte del proceso está finalizado, se han enviado las peticiones y se encarga el receptor
        //La sincronización con el proceso se hace mediante un semaforo
        printf("Me paro");
        sem_wait(&esperaRespuesta);
        printf("[Nodo %i]: He entrado en la sección crítica",identificador_nodo);
        sleep(rand() % 5 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
        printf("[Nodo %i]: He terminado la sección crítica",identificador_nodo);

        //Ahora notificamos la salida de la SC a los vecinos
        /*for (int i=1;i<NODOSVECINOS; i++) {
            Paquete peticion;
            peticion.estado=FINALIZADO;
            peticion.instruccion=ACK;
            peticion.id_nodo=nodos[0];
            peticion.num_ticket=ticketnum;
            peticion.id_proceso=pid;//Para futuro para poder direccionar en multiples procesos
            NetworkSend(red,nodos[i], &peticion); //TODO: Imprimir algun dato más desde esta función
        }*/ //Esto es para notificar a todos, mejor avisar solo a los nodos que dejamos esperando


        Paquete activacion;
        activacion.estado=NO_INTERESADO;
        activacion.id_nodo=nodos[0];
        activacion.id_proceso=pid;
        activacion.instruccion=ACK;
        activacion.num_ticket=ticketnum;
        // Recorrer la lista
        struct Nodo* actual = nodosenespera;
        while (actual != NULL) {
            printf("Notificando al nodo: %d ", actual->valor);
            NetworkSend(red,actual->valor,&activacion);
            actual = actual->siguiente;
        }
        estado = 0;

    }while (1); // Bucle para que funcione constantemente*/ 
    return 0;
}

