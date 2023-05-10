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
void* recepcion(void* args){
    //Tenemos que definir los tipos de cada uno de los paquetes.
    // Espera a recibir un mensaje en la cola de mensajes
    int acks = 0;
    while(1){
        printf("ESCUCHANDO CONEXIONES POR RED %i \n",red);

        Paquete recibido;


        printf("Voy a llamar a networkrcsv");
        networkrcv(red,nodos[0],&recibido);
        printf("Sali de networkrcsv");

        printf("ENTRO \n");
        printf("HA LLEGADO UN PAQUETE\n\n");

        printf("La prioridad es %i\n",recibido.prioridad);
        printf("La Instruccion es %i\n",recibido.instruccion);
        if(recibido.ticket>lastticket){
            lastticket=recibido.ticket;
            printf("Actualizo el numero de ticket con externo\n");
        }  // Si el ticket que recibo es mayor actualizo  

       
        if(recibido.instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO
            printf("Es una solicitud\n\n");
            int contarproces=contarProcesos(cola);
            fflush(stdout);
            printf("Contar procesos da como resultado");
            if(contarproces==0){
                //La cola está vacía, podemos contestar directamente
                printf("La cola está vacía, puedo dar paso a la solicitud de nodo %i",recibido.idNodo);
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
                printf("La cola NO está vacía, añado el proceso a la cola");

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
            printf("Es un ACK\n");

            

            sem_wait(&sem_protec_lista);
                printf("Se va a dar permiso al proceso %i\n",recibido.proceso);
                addACK(cola,recibido.proceso);//Añadir el ack
            sem_post(&sem_protec_lista);
            fflush(stdout);
            printf("Se ha añadido el ACK");

            int acks=ACKproceso(cola,recibido.proceso);
            if(acks==NODOSVECINOS-1){
                //Este proceso ya tiene todos los ack
                sem_post(&sem_prioridades_ACK[recibido.prioridad]);// Después en el wait ese deberán de revisar si tienen todos los permisos necesarios pero no es cosa del receptor
                printf("proceso despertadoc\n");
            }else{
                printf("El proceso no tiene todos los ACK's %i\n",acks);
            }

        }else if (recibido.instruccion==NACK){
            //No nos están dando el permiso para SC??
        }else{
            printf("Caso raro");
        }

    }

}

void * procesomutex(void *arg){
    struct Proceso yomismo;
    int *prioridad = (int *)arg;

    yomismo.creado= time(NULL);
    yomismo.idProceso = gettid();
    
    //int contadorschilo=1;
    //int valorSemaforoSC;
    //int valorSemaforoAvisoNodos;


    printf("[Proceso %d] -> Acabo de entrar en procesomutex\n", yomismo.idProceso);
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


            printf("[Proceso %d] -> Intentando entrar en SC...\n", yomismo.idProceso);
            yomismo.contACK=0;
            yomismo.idNodo=nodos[0];
            yomismo.pedirPermiso=1;
            yomismo.prioridad=(*prioridad);
            printf("Pto control");
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
                sem_wait(&sem_prioridades[yomismo.prioridad]);
            }

            cola->ejecucion=1;
            yomismo.instruccion=SOLICITUD;
            yomismo.estado=SOLICITANTE;

            if (cola->pedirPermiso){

                for (int i=1;i<NODOSVECINOS; i++){
                    yomismo.mtype=nodos[i];
                    printf("es aqui, la instruccion es %i\n\n\n",yomismo.instruccion);
                    NetworkSend(red,&yomismo,NULL);
                }
                printf("Hola como estás\n");

                bool rep=false;
                do {
                    printf("Espero los ACK");
                    sem_wait(&sem_prioridades_ACK[yomismo.prioridad]);
                    printf("ACK recibidos\n");
                    
                    rep= ((cola->idProceso==yomismo.idProceso)==false) || (cola->contACK!=NODOSVECINOS-1);
                    if(rep){
                        sem_post(&sem_prioridades_ACK[yomismo.prioridad]); //El despertar no era para mi
                    }

                    sleep(1);
                }while (rep);
            }
            
            yomismo.contACK=cola->contACK;
            
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
                printf("El siguiente proceso es tiene ticket %i\n",cola->ticket);
                sem_post(&sem_prioridades[cola->prioridad]);
                printf("Ha despertado?\n");
            }else if(contarProcesos(cola)!=0) {
                printf("EL SIGUIENTE PROCESO NO ESTA EN MI NODO\n");
                cola->mtype=cola->idNodo;
                NetworkSend(red,cola,NULL);
                necesariapeticion(cola,nodos[0]); //Agrega los boolean de necesaria petición
            }else{
                //El siguinte proceso esta esperando fuera de mi nodo
                printf("Cola terminada!\n");
            }
            yomismo.fin= time(NULL);
            
            agregarProceso(&historial, &yomismo);
            
            sleep(1);
    return NULL;
    
}
void sigint_handler(int sig) {
    //printf("\n\n\n\n");
    //printf("\nESTE NDOO HA ENTRADO EN LA SC UN TOTAL DE : %i   veces\n",contadorsc);
    printf("Se ha presionado Ctrl+C eliminando buzones....\n");
    if (msgctl(red, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    printf("Buzón eliminado.\n");

    //
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

    imprimirLista("historialdeejecucion.csv",historial);

    exit(EXIT_SUCCESS);
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

if(nodos[0]==10){

 
    int procesos =5;
    int prioridadrand[procesos];


    int i =0;
    do{
        pthread_t pthtest[procesos];

        for (int i =0; i<procesos;i++) {
            printf("Creo hilo\n");
            prioridadrand[i]=rand() % 3 + 1;
            pthread_create(&pthtest[i],NULL,(void *)procesomutex,(void *)&prioridadrand[i]);
        }

pthread_join(pthrecepcion, NULL); // Esperar a que el hilo termine
        for (int i = 0; i < procesos; i++) {
            pthread_join(pthtest[i], NULL); // Esperar a que el hilo termine
        }
        i++;
    }while(i<5);

}else{pthread_join(pthrecepcion, NULL); }
  sigint_handler(0);
    return 0;
}

void initparam(int argc, char *argv[]){
    //Parametros con las ID
    NODOSVECINOS=argc-1;
    //printf("Se van a iniciar %i nodos\n",NODOSVECINOS);

    //Parametros a entero
    for (int i = 1; i < argc; i++) {
        nodos[i-1] = atoi(argv[i]);
    }

    srand(time(NULL)); //Para que los aleatorios no sean siempre los mismos!

    //printf("Nodos vecinos son:\n");
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