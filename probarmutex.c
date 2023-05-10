/*ABOUT: Este es el programa creado para la primera entrega de clase por error. Consiste en un algoritmo para 1 proceso N nodos
Los nodos se definen como parámetros de entrada. Siendo tantos parámeteros como Nodos introducidos por parémetro.
IMPORTANTE: El primer parámetro siempre tiene que ser la ID del nodo
Para poder ejecutar el sript de inicialización autiomática de nodos compilar con nombre de salida multinodo.o*/

/*Contexto: Lanzamiento: En función del parámetro introducido ejecuta una función o otra para poder usar el mismo programa.
(Ahora mismo parece buena idea, igual no por mucho tiempo)*/
/*Ejemplo de lanzamiento: 
multiproceso.o RECEP ID_COLA_INTERNA ID_COLA_RED #########Lanza la aplicacioón receptor 
multiproceso.o CONSULTAS ID_COLA_INTERNA ID_COLA_RED ID_NODO [ID'S NODOS]*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef TIMER
#define TIMER
#include <time.h>
#endif
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>


#include "linkedlist.h"
#include "network.h"
#include "datatypes.h"

//gcc -o multiproceso.o multiproceso.c network.c linkedlist.c
///////////////////////////////////
#define PROBABILIDAD_ENTRADA 0.75
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01
#define NODOSVECINOS 3

int red =0;

Status estado = 0;

int nodos[100]; //IMPORTANTE en nodos[0] siempre está mi ID


int lastticket=15; // Este es el mayor número de ticket recibido.

sem_t sem_crear_hilos;

sem_t sem_consultas;
sem_t sem_SC;
sem_t sem_esperaAvisoNodos;
sem_t sem_saleSC;
sem_t sem_espera_ACK;
sem_t sem_avisar_nodos_en_espera;
sem_t sem_protec_var_estado;
sem_t sem_protec_lista;
sem_t sem_protec_ordenarLista;
sem_t sem_protec_lastticket;

sem_t sem_prioridades[5];

struct Proceso* cola = NULL;  
struct Proceso* historial = NULL;   

void initparam();
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
        
//        Paquete* recibido = networkrcv(red,nodos[0]);
        Paquete* recibido = NULL;

        if(recibido->instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO
            
/*
            if(contarProcesos==0){
                //La cola está vacía, podemos contestar directamente
            }else{
                //La cola no está vacía, tenemos que añadir a la cola el proceso externo y que luego cuando toque se conteste
            }

*/
            if(procesoSC(cola)==true){
                //Dejamos que pase el otro proceso:   MOTIVO --> nadie en mi nodo está en SC
                if(recibido->process.ticket>lastticket){
                    lastticket=recibido->process.ticket;
                }  // Si el ticket que recibo es mayor actualizo

              
                //En estos casos existe contienda.
            }else{ //Pasa el nodo con ticket menor, el otro

            }

        }else if(recibido->instruccion==ACK){
            //Nos están dando permiso para entrar SC
            //Primera comprobación, realmente, queremos?
            sem_wait(&sem_protec_lista);
            addACK(cola,recibido->process.idNodo);//Añadir el ack
            sem_post(&sem_protec_lista);
            if(ACKproceso(cola,recibido->process.idNodo)==NODOSVECINOS-1){
                //Este proceso ya tiene todos los ack
                sem_post(&sem_prioridades);// Después en el wait ese deberán de revisar si tienen todos los permisos necesarios pero no es cosa del receptor
            }            
        }else if (recibido->instruccion==NACK){
            //No nos están dando el permiso para SC??
        }

    }

}

void * procesomutex(int* prioridad){
    struct Proceso yomismo;

    yomismo.creado= time(NULL);
    yomismo.idProceso = gettid();
    
    //int contadorschilo=1;
    //int valorSemaforoSC;
    //int valorSemaforoAvisoNodos;

    int nodos[1];
    nodos[0]=1;
    printf("[Proceso %d] -> Acabo de entrar en procesomutex\n", yomismo.idProceso);
    
    //do{

        //Aleatorizar la entrada en SC
        /*
            bool aleatoriaentrada=true;
            printf("[Proceso %d] -> Intentando entrar en la seccion critica...\n", hilo_pid);
            do {

                aleatoriaentrada = (double)rand() / RAND_MAX < PROBABILIDAD_ENTRADA;
                if (!aleatoriaentrada) {
                    // Si no quiero entrar espero
                    
                    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
                }else{}
            }while (!aleatoriaentrada); // 0 No interesado 1 SOLICITANTE*/


            printf("[Proceso %d] -> Intentando entrar en SC...\n", yomismo.idProceso);
            yomismo.contACK=0;
            yomismo.idNodo=nodos[0];
            yomismo.prioridad=prioridad;
            
            yomismo.ticket=lastticket+rand() % 5;
            //Actualizo el numero de ticket minimo
            sem_wait(&sem_protec_lastticket);
                lastticket=yomismo.ticket;
            sem_post(&sem_protec_lastticket);


            //Me pongo a la cola y ordeno
            sem_wait(&sem_protec_lista);
                agregarProceso(&cola,&yomismo);
                printf("[Nodo %d] -> Agregado proceso %d con prioridad: %d, Ticket: %d e idNodo: %d\n", yomismo.idNodo, yomismo.idProceso, yomismo.prioridad, yomismo.ticket, yomismo.idNodo);
                imprimirLista("Agrega.csv",cola);
                sem_wait(&sem_protec_ordenarLista);
                    ordenarCola(&cola);
                sem_post(&sem_protec_ordenarLista);
            sem_post(&sem_protec_lista);

            printf("[Proceso %d] -> Mi prioridad es %d y el de la cabeza es %d. RESULTADO %d\n", yomismo.idProceso, yomismo.prioridad, cola->prioridad, esIgual(cola, &yomismo));
            if(esIgual(cola, &yomismo)){
                //Tengo permiso para entrar en SC
                printf("[Proceso %d] -> Tengo permisos\n", yomismo.idProceso);  
            }else{
                //Me espero
                printf("[Proceso %d] -> NO Tengo permisos\n", yomismo.idProceso);
                sem_wait(&sem_prioridades[yomismo.prioridad]);
                printf("[Proceso %d] -> He despertado!\n", yomismo.idProceso);
            }
            cola->ejecucion=1;

            
            //SECCION CRITICA

            printf("Acabo de llegar a SC con Ticket: %i \n",yomismo.ticket);
            yomismo.inicio= time(NULL);
            int tiempoespera=rand() % 1 + 2;
            sleep(tiempoespera);
            yomismo.atendido= time(NULL);       
            usleep(500);

            
            
            sem_wait(&sem_protec_lista);
                eliminarCabeza(&cola);
                sem_wait(&sem_protec_ordenarLista);
                    if(cola != NULL) ordenarCola(&cola);
                sem_post(&sem_protec_ordenarLista);
            sem_post(&sem_protec_lista);

            printf("Quedan en cola %i procesos\n",contarProcesos(cola));


            if(compararIdNodo(cola,nodos[0])){
                //El sigueinte proceso está esperando en mi nodo
                printf("EL SIGUIENTE PROCESO ESTA EN MI NODO\n");
                printf("El siguiente proceso es de prioridad %i\n",cola->prioridad);
                printf("El siguieºnte proceso es tiene ticket %i\n",cola->ticket);
                sem_post(&sem_prioridades[cola->prioridad]);
                printf("Ha despertado?\n");
            }else if(contarProcesos(cola)!=0) {
                printf("EL SIGUIENTE PROCESO NO ESTA EN MI NODO\n");
            }else{
                //El siguinte proceso esta esperando fuera de mi nodo
                printf("Cola terminada!\n");
            }
            /*                printf("[Proceso %d] -> EL SIGUIENTE PROCESO ESTA EN MI NODO\n", hilo_pid);
                printf("[Proceso %d] -> El siguiente proceso es de prioridad %i\n",hilo_pid, cola->prioridad);
                printf("[Proceso %d] -> El sigueinte proceso es tiene ticket %i\n",hilo_pid, cola->ticket);

                sem_post(&sem_prioridades[cola->prioridad]);
                printf("[Proceso %d] -> Ha despertado?\n", hilo_pid);
            }else{
                //El siguinte proceso esta esperando fuera de mi nodo
                printf("[Proceso %d] -> EL SIGUIENTE PROCESO NO ESTA EN MI NODO\n", hilo_pid);*/

            yomismo.fin= time(NULL);
            
            agregarProceso(&historial, &yomismo);
            
            sleep(1);
        //}while(1);

    return NULL;
    
}

int main(int argc, char *argv[]) {
    printf("Hola\n");
    srand(time(NULL)); 
    initparam();
    int procesos =10;
    pthread_t pthtest[procesos];


    //pthread_t pthrecepcion;
    //pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);

    for (int i =0; i<procesos;i++) {
        printf("Creo hilo\n");
        int prioridadrand=rand() % 3 + 1;
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,prioridadrand);
    }
    
    for (int i = 0; i < procesos; i++) {
        pthread_join(pthtest[i], NULL); // Esperar a que el hilo termine
    }
    imprimirLista("historialdeejecucion.csv",historial);
    return 0;
}

void initparam(){
    //INICIALIZACIÓN DE SEMAFOROS
    sem_init(&sem_SC,0,1);
    sem_init(&sem_esperaAvisoNodos,0,1);
    sem_init(&sem_espera_ACK,0,0);
    sem_init(&sem_consultas,0,5);
    sem_init(&sem_saleSC,0,0);
    sem_init(&sem_avisar_nodos_en_espera,0,0);
    sem_init(&sem_protec_var_estado,0,1);
    sem_init(&sem_protec_lista,0,1);
    sem_init(&sem_protec_lastticket,0,1);
    sem_init(&sem_protec_ordenarLista,0,1);

    for (int i = 0; i < 5; i++) {
        sem_init(&sem_prioridades[i], 0, 0);
    }
}