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
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include "network.h"
#include "datatypes.h"
#include "linkedlist.h"
#include "multiproceso.h"

//gcc -o multiproceso.o multiproceso.c network.c linkedlist.c
///////////////////////////////////
#define PROBABILIDAD_ENTRADA 0.75
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01
//#define NODOSVECINOS 10


//Memoria y variables del proceso
int identificador_nodo = 0;
int red =0;
sem_t esperaRespuesta;
Status estado = 0;
int contadorsc=0;

int lastticket=78; // Este es el mayor número de ticket recibido.
int ticketnum;  // Este es el numero de ticket que yo estoy usando
struct Nodo* nodosenespera = NULL; //Lista con los nodos en espera
int NODOSVECINOS = 0;

char* estadostring[3] = { "NO_INTERESADO", "SOLICITANTE", "FINALIZADO"};
char* tipostring[3] = { "SOLICITUD", "ACK", "NACK"};

int nodos[100]; //IMPORTANTE en nodos[0] siempre está mi ID

int consultasActivas = 1;
int consultasEnSC = 0;

int colaConsultas = 0;

sem_t sem_crear_hilos;

sem_t sem_consultas;
sem_t sem_SC;
sem_t sem_esperaAvisoNodos;
sem_t sem_saleSC;
sem_t sem_espera_ACK;
sem_t sem_avisar_nodos_en_espera;
sem_t sem_protec_var_estado;

void* procesoHilo(int * param);
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
    int valorSemaforoSC;
    int valorSemaforoAvisoNodos;
    int procesosesperandoACK;
    //printf("Escuchando... por la red: %i\n",red);
    while(1){
        
        Paquete* recibido = networkrcv(red,nodos[0]);
        printf("[Nodo %i][RECEPTOR] Nuevo paquete recibido: Mi estado es %s y he recibido de tipo %s. Tiene ticket %i y yo tengo ticket %i \n",nodos[0],estadostring[estado],tipostring[recibido->instruccion],recibido->num_ticket,lastticket);
        sem_getvalue(&sem_esperaAvisoNodos, &valorSemaforoAvisoNodos);
        sem_getvalue(&sem_SC, &valorSemaforoSC);
        sem_getvalue(&sem_espera_ACK, &procesosesperandoACK);

        printf("[Nodo %i][RECEPTOR] El valor del semaforo SC es: %i el valor del SemaforoAvisoNodos es %i. El valor de semaforos ACK %i\n",nodos[0],valorSemaforoSC,valorSemaforoAvisoNodos,procesosesperandoACK);

        if(recibido->instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO
            //Primero es necesario conocer si hay contienda mediante mi estado
            
            if(estado==NO_INTERESADO){
                //Dejamos que pase el otro proceso:   MOTIVO --> //No estoy interesado
                if(recibido->num_ticket>lastticket){
                    lastticket=recibido->num_ticket;
                    //printf("####### Nuevo numero de ticket minimo %i.\n",lastticket);
                }  // Si el ticket que recibo es mayor actualizo

                //Mando el ACK
                NetworkSend(red,nodos[0], recibido->id_nodo, NO_INTERESADO,0,ACK,lastticket);



    //En estos casos existe contienda.
            }else if (estado==SOLICITANTE && (recibido->num_ticket<ticketnum) ){ //Pasa el nodo con ticket menor, el otro
                //Dejamos que pase el otro proceso:   MOTIVO --> //Su ticket es menor
                //Mando el ACK
                NetworkSend(red,nodos[0],recibido->id_nodo, SOLICITANTE,0,ACK,lastticket);


            }else if(estado==SOLICITANTE && (recibido->num_ticket==ticketnum) ){ //Tenemos el mismo numero de ticket, resolvemos con el ID nodo
                if(recibido->id_nodo<nodos[0]){
                    //Entra el nodo con ID menor, le mando el ACK
                    NetworkSend(red,nodos[0], recibido->id_nodo, SOLICITANTE,0,ACK,lastticket);

                }else{
                    //No autoricé al nodo, tengo que despertarlo cuando termine
                    agregarNodo(&nodosenespera,recibido->id_nodo);
                    //printf("[Nodo %i] Nueva solicitud agregada a la cola de pendientes.\n",nodos[0]);
                }

            }else if(estado==SOLICITANTE && (recibido->num_ticket>ticketnum)){
                //Tengo un ticket menor al del solicitante asi que lo agrego a la lista
                agregarNodo(&nodosenespera,recibido->id_nodo);
                //printf("[Nodo %i] Nueva solicitud agregada a la cola de pendientes.\n",nodos[0]);

            }
        }else if(recibido->instruccion==ACK){
            //Nos están dando permiso para entrar SC
            //Primera comprobación, realmente, queremos?
            //printf("Estoy en este else");
            if(estado==SOLICITANTE){ // Confirmo que quiero entrar
                //Cuando puedo entar a SC? Cuando tengo permiso de todos los nodos
                acks++; //Hemos recibido un ACK
                //printf("[Nodo %i] Nuevo ACK recibido. %i\n",nodos[0],acks);
                if (acks==NODOSVECINOS-1){
                    //Tenemos los permisos necesarios para acceder a SC
                    //Aviso al proceso de que pase
                    sem_post(&sem_espera_ACK);
                    //printf("[Nodo %i] RECEPTOR: Se ha notificado al proceso de que tiene permisos para entar.\n",nodos[0]);
                    acks=0;
                }
            }
        }else if (recibido->instruccion==NACK){
            //No nos están dando el permiso para SC??
        }

    }

}


void sigint_handler(int sig) {
    printf("\n\n\n\n");
    printf("\nESTE NDOO HA ENTRADO EN LA SC UN TOTAL DE : %i   veces\n",contadorsc);
    printf("Se ha presionado Ctrl+C eliminando buzones....\n");
    if (msgctl(red, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    printf("Buzón eliminado.\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    //Signal de salida
    //printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);

    red=msgget(99999, IPC_CREAT | 0777);
    //printf("red establecida: %i\n",red);

    initparam(argc, argv);

    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);   
    pthread_t pthtest[251];
    for (int i =0; i<2; i++) {
        //printf("Creo hilo");
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,NULL);   
    }
   
    while (1) {
    
    }
    return 0;
}


void * procesomutex(int * param){
    srand(time(NULL)); //Para que los aleatorios no sean siempre los mismos!

    //pid_t hilo_pid = getpid();
    pid_t hilo_pid = gettid();
    
    int contadorschilo=0;
    int valorSemaforoSC;
    int valorSemaforoAvisoNodos;

    do{

        //Aleatorizar la entrada en SC
            bool aleatoriaentrada=true;
            do {

                aleatoriaentrada = (double)rand() / RAND_MAX < PROBABILIDAD_ENTRADA;
                if (!aleatoriaentrada) {
                    // Si no quiero entrar espero
                    usleep(500000); // Espera 500000 microsegundos (medio segundo)
                }else{}
            }while (!aleatoriaentrada); // 0 No interesado 1 SOLICITANTE

            //PASOS NECESARIOS PARA ENTRAR EN LA SC
            // 1 NOTIFICAR
            // 2 ESPERAR RESPUESTA
            // 3 EVALUAR RESPUESTA
            // SI OK: ENTRO -> Esta espera al OK se puede hacer con un semaforo

        //Comprobar si alguien en el nodo está en SC

            //Comprobar si hay nodos esperando por una respuesta
                bool necesariasolicitud=true;
                sem_getvalue(&sem_SC, &valorSemaforoSC);

                int cantidadnodosesperando=contarNodos(nodosenespera);
                printf("[Nodo %i] Nodos en espera: %i",nodos[0],cantidadnodosesperando);
                printf("[Nodo %i] Valor semaforo SC: %i",nodos[0],valorSemaforoSC);

                if(cantidadnodosesperando>0){
                    //printf("[Nodo %i]IF 1.1",nodos[0]);
 
                    //Hay más nodos en espera no puedo entrar en SC sin solicitud
                    printf("[Proceso %d] -> He entrado en cantidad Nodos esperando %d\n", hilo_pid, cantidadnodosesperando);
                    necesariasolicitud=true;
                    sem_wait(&sem_esperaAvisoNodos);

                    //Antes de que llegue un post aquí se ha cambiado el estado del nodo para pasar de nuevo a no interesado
                    sem_wait(&sem_protec_var_estado);
                    estado=SOLICITANTE;
                    printf("Cambio mi estado a solicitante");
                    sem_post(&sem_protec_var_estado);
                }else if(estado==SOLICITANTE){
                    printf("[Nodo %i]IF 1.2",nodos[0]);
                    //Mi nodo ya está en SC y no hay solicitudes entrantes nuevas
                    //No es necesario pedir permiso nuevamente para entrar en SC
                    necesariasolicitud=false;
                    if(estado!=SOLICITANTE){ //Esto es una medida de seguridad un poco tonta...
                        sem_wait(&sem_protec_var_estado);
                        estado=SOLICITANTE;
                        printf("Cambio mi estado a solicitante");
                        sem_post(&sem_protec_var_estado);
                    }
                }else{
                    printf("[Nodo %i]IF 1.3",nodos[0]);
                    //En este caso tiene pinta de que soy el primer hilo interesado entrar en SC o el primero con acceso
                    necesariasolicitud=true;
                    sem_wait(&sem_protec_var_estado);
                    estado=SOLICITANTE;
                    //printf("Cambio mi estado a Solicitante");
                    sem_post(&sem_protec_var_estado);

                }
            
            
            printf("\n[Nodo %i Hilo %i] Espero en semaforo SC del nodo.\n",nodos[0],hilo_pid);
            sem_wait(&sem_SC);
            printf("\n[Nodo %i Hilo %i] Nadie en mi nodo tiene SC, me ha liberado del semSC.\n",nodos[0],hilo_pid);

            //En caso de ser necesario se notifica a los nodos vecinos mi deseo de entar en SC
            if(necesariasolicitud){
                printf("[Nodo %i]IF 2",nodos[0]);
                ticketnum = lastticket + rand() % NODOSVECINOS + 5;
                
                for (int i=1;i<NODOSVECINOS; i++){
                    NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,hilo_pid,SOLICITUD,ticketnum);
                }
                printf("\n[Nodo %i Hilo %i] Nodos vecinos notificados, espero respuestas.\n",nodos[0],hilo_pid);
                if(NODOSVECINOS>1)sem_wait(&sem_espera_ACK); //Nos manda el permiso el receptor
                printf("\n[Nodo %i Hilo %i] ACK's recibidos.\n",nodos[0],hilo_pid);

            }

            //SECCION CRITICA
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            int tiempoespera=rand() % 2 + 5;
            //sleep(tiempoespera);
            tiempoespera=tiempoespera * 10000;
            printf("%i:%i:%i.%i,%i,%i,Entra,%i,%i\n",t->tm_hour,t->tm_min,t->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,contadorsc,contadorschilo);
            usleep(5000000); // Espera 500000 microsegundos (medio segundo); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }

            
            now = time(NULL);
            struct tm *t2 = localtime(&now);
            printf("%i:%i:%i.%i,%i,%i,Sale,%i,%i\n",t->tm_hour,t->tm_min,t->tm_sec,(int) clock() % 1000,nodos[0],hilo_pid,contadorsc,contadorschilo);
            contadorsc++;
            contadorschilo++;

            sem_getvalue(&sem_esperaAvisoNodos, &valorSemaforoAvisoNodos);
            sem_getvalue(&sem_SC, &valorSemaforoSC);
            cantidadnodosesperando=contarNodos(nodosenespera);
            
            printf("\n[Nodo %i Hilo%i] Valor sem SC %i.\n",nodos[0],hilo_pid,valorSemaforoSC);
            printf("\n[Nodo %i Hilo%i] Valor sem Avisa nodos %i.\n",nodos[0],hilo_pid,valorSemaforoAvisoNodos);
            printf("\n[Nodo %i Hilo%i] Nodos en cola. %i\n",nodos[0],hilo_pid,cantidadnodosesperando);
            

            
            if((cantidadnodosesperando!=0 && valorSemaforoSC==0)||valorSemaforoSC==0){
                    printf("[Nodo %i]IF 3.1",nodos[0]);
                    //Hay nodos esperando y ha terminado la ráfaga de procesos
                    sem_wait(&sem_protec_var_estado);
                    estado=NO_INTERESADO;
                    printf("Cambio mi estado a no interesado");
                    sem_post(&sem_protec_var_estado);

                    sem_getvalue(&sem_esperaAvisoNodos, &valorSemaforoAvisoNodos);
                    if(valorSemaforoAvisoNodos<1)sem_post(&sem_esperaAvisoNodos);


                        struct Nodo* actual = nodosenespera;
                        while (actual != NULL) {
                            printf("Notificando al nodo que ahora si puede entrar: %d \n", actual->valor);
                    
                            NetworkSend(red,nodos[0], actual->valor, NO_INTERESADO,hilo_pid, ACK, ticketnum);
                            actual = actual->siguiente;
                        }
                        
                        borrarLista(&nodosenespera);
                    sem_post(&sem_SC);
            }else{
                printf("[Nodo %i]IF 3.2",nodos[0]);
                //Hay hilos esperando en una posición donde podrían (o  no) pedir permiso
                sem_post(&sem_SC);
            }

           
            
            lastticket=ticketnum;
            sleep(1);
    }while(1);
    
}


void initparam(int argc, char *argv[]){
    //Parametros con las ID
    NODOSVECINOS=argc-1;
    //printf("Se van a iniciar %i nodos\n",NODOSVECINOS);

    //Parametros a entero
    for (int i = 1; i < argc; i++) {
        nodos[i-1] = atoi(argv[i]);
    }


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
    

}
