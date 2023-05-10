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


int NODOSVECINOS = 0;

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
sem_t sem_prioridades_ACK[5];

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

void guardalog() {

    char fichero[300];
    char *pid;
    char *str1 = "historialporordendeejecucion_";
    char *str2 = ".csv";

    if(asprintf(&pid, "%d",nodos[0]) == -1);
    else {
        strcat(strcpy(fichero, str1), pid);
        strcat(fichero, str2);
    }
    imprimirLista(fichero, historial);
    printf("Guardado registro log\n");

    return;
}
void sigint_handler(int sig) {

    guardalog();
    printf("Se ha presionado Ctrl+C eliminando buzones....\n");
    if (msgctl(red, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    printf("Buzón eliminado.\n");

    

    exit(EXIT_SUCCESS);
}
void* recepcion(void* args){
    //Tenemos que definir los tipos de cada uno de los paquetes.
    // Espera a recibir un mensaje en la cola de mensajes
    int acks = 0;
            printf("[RECEPTOR %i] ESCUCHANDO CONEXIONES POR RED %i \n",nodos[0],red);

    while(1){

        Paquete recibido;
        networkrcv(red,nodos[0],&recibido);

        if(recibido.ticket>lastticket){
            lastticket=recibido.ticket;
        }  // Si el ticket que recibo es mayor actualizo  

       
        if(recibido.instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO

            int contarproces=contarProcesos(cola);
            fflush(stdout);
            if(contarproces==0){
                //La cola está vacía, podemos contestar directamente
                recibido.mtype=recibido.idNodo;
                recibido.instruccion=ACK;
                recibido.estado=NO_INTERESADO;
                NetworkSend(red,NULL,&recibido);

                struct Proceso mensajeRecibido;
                mensajeRecibido.idNodo = recibido.idNodo;
                mensajeRecibido.ticket = recibido.ticket;
                mensajeRecibido.prioridad = recibido.prioridad;
                mensajeRecibido.idProceso = recibido.proceso;
                mensajeRecibido.contACK = -10;
                agregarProceso(&historial, &mensajeRecibido);
            }else{
                //La cola no está vacía, tenemos que añadir a la cola el proceso externo y que luego cuando toque se conteste

                sem_wait(&sem_protec_lista);
                    addACK(cola,recibido);//Añadir el ack
                    ordenarCola(cola);
                sem_post(&sem_protec_lista);
            }


            if(procesoSC(cola)==true){
                //Dejamos que pase el otro proceso:   MOTIVO --> nadie en mi nodo está en SC
    
              
                //En estos casos existe contienda.
            }else{ //Pasa el nodo con ticket menor, el otro

            }

        }else if(recibido.instruccion==ACK){
            //Nos están dando permiso para entrar SC
            //Primera comprobación, realmente, queremos?

            

            sem_wait(&sem_protec_lista);
                addACK(cola,recibido.proceso);//Añadir el ack
            sem_post(&sem_protec_lista);

            int acks=ACKproceso(cola,recibido.proceso);
            if(acks==NODOSVECINOS-1){
                //Este proceso ya tiene todos los ack
                sem_post(&sem_prioridades_ACK[recibido.prioridad]);// Después en el wait ese deberán de revisar si tienen todos los permisos necesarios pero no es cosa del receptor
            }

        }else if (recibido.instruccion==NACK){
            //No nos están dando el permiso para SC??
        }else{
            printf("Caso raro");
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


    printf("[Proceso %d] -> Soy un proceso de tipo %i\n", yomismo.idProceso,prioridad);
        //Aleatorizar la entrada en SC
            bool aleatoriaentrada=true;
            printf("[Proceso %d] -> Intentando entrar en la seccion critica...\n", yomismo.idProceso);
            do {

                aleatoriaentrada = (double)rand() / RAND_MAX < PROBABILIDAD_ENTRADA;
                if (!aleatoriaentrada) {
                    // Si no quiero entrar espero
                    
                    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
                }else{}
            }while (!aleatoriaentrada); // 0 No interesado 1 SOLICITANTE

            yomismo.contACK=0;
            yomismo.idNodo=nodos[0];
            yomismo.pedirPermiso=1;
            yomismo.prioridad=prioridad;
            yomismo.ticket=lastticket+rand() % 5;

            //Actualizo el numero de ticket minimo
            sem_wait(&sem_protec_lastticket);
                lastticket=yomismo.ticket;
            sem_post(&sem_protec_lastticket);


            //Me pongo a la cola y ordeno
            sem_wait(&sem_protec_lista);
                agregarProceso(&cola,&yomismo);
                sem_wait(&sem_protec_ordenarLista);
                    ordenarCola(&cola);
                sem_post(&sem_protec_ordenarLista);
            sem_post(&sem_protec_lista);

            if(esIgual(cola, &yomismo)){
                //Tengo permiso para entrar en SC
            }else{
                sem_wait(&sem_prioridades[yomismo.prioridad]);
            }
            printf("[Proceso %d] -> Es mi turno de entar en SC\n", yomismo.idProceso);

            cola->ejecucion=1;
            yomismo.instruccion=SOLICITUD;
            yomismo.estado=SOLICITANTE;

            if (cola->pedirPermiso){

                for (int i=1;i<NODOSVECINOS; i++){
                    yomismo.mtype=nodos[i];
                    NetworkSend(red,&yomismo,NULL);
                }

                bool rep=false;
                do {
                    printf("[Proceso %d] -> Esperando ACK's\n", yomismo.idProceso);
                    sem_wait(&sem_prioridades_ACK[yomismo.prioridad]);                 
                    rep= ((cola->idProceso==yomismo.idProceso)==false) || (cola->contACK!=NODOSVECINOS-1);
                    if(rep){
                        sem_post(&sem_prioridades_ACK[yomismo.prioridad]); //El despertar no era para mi
                    }else{
                        printf("[Proceso %d] -> ACK's recibidos.\n", yomismo.idProceso);

                    }
                    //TODO: ojo aquí
                    sleep(1);
                }while (rep);
            }
            
            yomismo.contACK=cola->contACK;
            
            //SECCION CRITICA

            printf("[Proceso %d] -> Entrando en SC con ticket\n", yomismo.idProceso,yomismo.ticket);

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

            if(compararIdNodo(cola,nodos[0])){
                //El sigueinte proceso está esperando en mi nodo
                cola->pedirPermiso=0;
                sem_post(&sem_prioridades[cola->prioridad]);
            }else if(contarProcesos(cola)!=0) {
                //El siguiente proceso esta en otro nodo
                do{
                    cola->mtype=cola->idNodo;
                    NetworkSend(red,cola,NULL);
                    eliminarCabeza(&cola);
                    if(cola->idNodo==nodos[0]){
                        sem_post(&sem_prioridades[cola->prioridad]);
                    }else if(contarProcesos(cola)==0){
                        printf("[Proceso %d] -> Se ha finalizado la cola de procesos\n", yomismo.idProceso,yomismo.ticket);
                        agregarProceso(&historial, &yomismo);
                        guardalog();
                        return NULL; 
                    }
                }while(cola->idNodo!=nodos[0] || contarProcesos(cola)!=0 );

            }else{
                //Cola terminada
                printf("[Proceso %d] -> Se ha finalizado la cola de procesos\n", yomismo.idProceso,yomismo.ticket);
                agregarProceso(&historial, &yomismo);
                guardalog();
                return NULL; 
            }
            yomismo.fin= time(NULL);
            
            agregarProceso(&historial, &yomismo);
            
            sleep(1);
    return NULL;
    
}


int main(int argc, char *argv[]) {
    //Signal de salida
    //printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);


    red=msgget(8888, IPC_CREAT | 0777);


    srand(time(NULL)); 
    initparam(argc, argv);
    

    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);



 
    int procesos =1;


    int i =0;
    do{
        pthread_t pthtest[procesos];

        for (int i =0; i<procesos;i++) {
            int prioridadrand=rand() % 3 + 1;
            pthread_create(&pthtest[i],NULL,(void *)procesomutex,prioridadrand);
        }

        pthread_join(pthrecepcion, NULL); // Esperar a que el hilo termine
        for (int i = 0; i < procesos; i++) {
            pthread_join(pthtest[i], NULL); // Esperar a que el hilo termine
        }
        i++;
    }while(i<5);

  sigint_handler(0);


    return 0;
}

void initparam(int argc, char *argv[]){
    //Parametros con las ID
    NODOSVECINOS=argc-1;

    //Parametros a entero
    for (int i = 1; i < argc; i++) {
        nodos[i-1] = atoi(argv[i]);
    }

    srand(time(NULL)); //Para que los aleatorios no sean siempre los mismos!


    for (int i=0; i<NODOSVECINOS; i++) {
        //printf("ID de nodo %i: %i \n",i,nodos[i]);
    }

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