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
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>


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
int contadorsc=1;

int lastticket=78; // Este es el mayor número de ticket recibido.
int ticketnum;  // Este es el numero de ticket que yo estoy usando
struct Nodo* nodosenespera = NULL; //Lista con los nodos en espera
int NODOSVECINOS = 0;

int nodos[100]; //IMPORTANTE en nodos[0] siempre está mi ID

int consultasActivas = 1;
int consultasEnSC = 0;

int colaConsultas = 0;

sem_t sem_crear_hilos;
sem_t sem_procesos;

sem_t sem_consultas;
sem_t sem_solicitaSC;
sem_t sem_saleSC;


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
    printf("Escuchando... por la red: %i\n",red);
    while(1){
        
        fflush(stdout);
        Paquete* recibido = networkrcv(red,nodos[0]);
        
        
        if(recibido->instruccion==SOLICITUD){
            //NOS HA LLEGADO UNA SOLICITUD DE UN NODO
            //Primero es necesario conocer si hay contienda mediante mi estado
            if(estado==0){
                //Dejamos que pase el otro proceso:   MOTIVO --> //No estoy interesado
                if(recibido->num_ticket>lastticket){
                    lastticket=recibido->num_ticket;
                    printf("####### Nuevo numero de ticket minimo %i.\n",lastticket);
                }  // Si el ticket que recibo es mayor actualizo

                //Mando el ACK
                NetworkSend(red,nodos[0], recibido->id_nodo, NO_INTERESADO,0,ACK,0);



    //En estos casos existe contienda.
            }else if (estado==1 && (recibido->num_ticket<ticketnum) ){ //Pasa el nodo con ticket menor, el otro
                //Dejamos que pase el otro proceso:   MOTIVO --> //Su ticket es menor
                //Mando el ACK
                NetworkSend(red,nodos[0],recibido->id_nodo, SOLICITANTE,0,ACK,0);


            }else if(estado==1 && (recibido->num_ticket==ticketnum) ){ //Tenemos el mismo numero de ticket, resolvemos con el ID nodo
                if(recibido->id_nodo<nodos[0]){
                    //Entra el nodo con ID menor, le mando el ACK
                    NetworkSend(red,nodos[0], recibido->id_nodo, SOLICITANTE,0,ACK,0);

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
                printf("Nuevo ACK recibido. %i\n",acks);
                if (acks==NODOSVECINOS-1){
                    //Tenemos los permisos necesarios para acceder a SC
                    //Aviso al proceso de que pase
                    sem_post(&esperaRespuesta);
                    printf("RECEPTOR: Se ha notificado al proceso de que tiene permisos para entar.\n");
                    acks=0;
                }
            }
        }else if (recibido->instruccion==NACK){
            //No nos están dando el permiso para SC??
        }

    }

}

void sigusr1_handler(int sig, siginfo_t *si, void *unused) {
    printf("Señal SIGUSR1 recibida valor %d\n", si->si_value.sival_int);
    // Aquí puedes procesar los datos recibidos
}
void sigusr2_handler(int sig, siginfo_t *si, void *unused) {
    printf("Señal SIGUSR2 recibida con valor %d\n", si->si_value.sival_int);
    // Aquí puedes procesar los datos recibidos
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
void ayuda() {
    printf("Uso:\n");
    printf("multiproceso.o RECEP ID_COLA_INTERNA ID_COLA_RED\n\n");
    printf("multiproceso.o TIPO ID_COLA_INTERNA ID_COLA_RED ID_NODO [ID'S NODOS]\n");
    printf("  TIPO: El tipo de proceso a lanzar: consultas,reservas...\n");
    printf("  ID_COLA_INTERNA:  Identificador del buzón interno de sincronizacion.\n");
    printf("  ID_COLA_RED:      Identificador de la cola sobre la que se simula la red\n");
    printf("  ID_NODO:          Identificador del nodo donde se ejecuta\n");
    printf("  [ID'S NODOS]:     Identificadores de nodos vecinos\n");
    // Agrega más información de ayuda según sea necesario
}
int main(int argc, char *argv[]) {

    //Parámetros
    if (argc < 4 || (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        // Muestra la ayuda y sale del programa
        ayuda();
        return 0;
    }

    //Signal de salida
    printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);

    //Asignacion de señal USR1
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Error al instalar manejador de señal");
        return 1;
    }
    //Asignacion de señal USR2
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr2_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Error al instalar manejador de señal");
        return 1;
    }

    //ES necesario volver a meter esto en un hilo
    //EJECUTO EL RECEPTOR
    //Creación del buzón del nodo
    int proj_id=52; //TODO: Esto hay que parametrizarlo
    key_t key=ftok("/bin/ls",proj_id);
    red=msgget(99999, IPC_CREAT | 0777);
    //printf("red establecida: %i\n",red);
    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);   

    recepcion();


    if(strcasecmp(argv[1], "RECEP") == 0){
        //EJECUTO EL RECEPTOR
            //Creación del buzón del nodo
            //int proj_id=52; //TODO: Esto hay que parametrizarlo
            //key_t key=ftok("/bin/ls",proj_id);
            red=msgget(99999, IPC_CREAT | 0777);
            //printf("red establecida: %i\n",red);
            recepcion();

   }else if(strcasecmp(argv[1], "consultas") == 0){
        //EJECUTO EL SERVICIO DE CONSULTAS
        initparam(argc, argv); //INICIALIZO LOS PARÁMETROS DEL PROGRAMA
        consultas();
   }else if(strcasecmp(argv[1], "reservas") == 0){
        //EJECUTO EL SERVICIO DE CONSULTAS
        initparam(argc, argv); //INICIALIZO LOS PARÁMETROS DEL PROGRAMA
        consultas();
   }else{//MUESTRO AYUDA
        ayuda();
        return 0;
   }
  
    return 0;
}

void * procesoHilo(int * param){
    pid_t pid = getpid();

// TODO: por ahora solo consultas. Despues coge el param para crear otro tipo de procesos
    int procesoPrio = 5;
    //int *pcolaConsultas = &colaConsultas;
    sem_t* sem_proceso;
    //Si el proceso no es una consulta, se desactiva las consultas para poder escribir.
    if(procesoPrio != 5){
        consultasActivas = 0;
        sem_proceso = &sem_consultas;
    }

    printf("Creado proceso de prioridad: %i\n", procesoPrio);
    sem_post(&sem_crear_hilos);

    sleep(5);

    if(procesoPrio != 5){
        sem_post(&sem_solicitaSC);
    }

    if(procesoPrio == 5 && consultasActivas == 1 && consultasEnSC == 1){
        //El proceso creado es CONSULTA, las consultas están ACTIVAS y hay consultas en SC, continua a la SC
    }else{
        //Esperamos a que el nodo nos de permiso
        sem_wait(sem_proceso);
    }


    printf("PROCESO con prioridad: %i entra a la SC\n", procesoPrio);

    if(procesoPrio == 5){
        consultasActivas = 1;
        consultasEnSC = 1;
    }

    if(procesoPrio == 5 && colaConsultas != 0){
        sem_post(&sem_solicitaSC);
    }

    sleep(7);

    printf("PROCESO con prioridad:%i sale de la SC\n", procesoPrio);

    sem_post(&sem_saleSC);

    consultasEnSC = 0;

    printf("FIN PROCESO PRIO: %i de PID: %i", procesoPrio, pid);

    pthread_exit(NULL);

}


// CREA numProcesos HILOS de PROCESOS con prioridad prioProceso
// prioProceso = 5 -> CONSULTAS
// prioProceso = 4 -> DEMÁS
int creaHiloProceso(int prioProceso, int numProcesos){

    pthread_t procesoHilos[1000];

    for(int i = 0; i < numProcesos; i++){

        pthread_create(&procesoHilos[i],NULL,(void *)procesoHilo,(void *)&prioProceso);
        sem_wait(&sem_crear_hilos);
    }

    return 0;

}

void consultas(){
    pid_t pid = getpid();
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

            creaHiloProceso(5,3);
            sem_post(&sem_consultas);
            sem_wait(&sem_saleSC);
            printf("Va a entrar en la SC...\n");
            fflush(stdout);

            //Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
            //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
            ticketnum = lastticket + rand() % NODOSVECINOS + 5;
            

            
            //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

            for (int i=1;i<NODOSVECINOS; i++){
                fflush(stdout);
                NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,pid,SOLICITUD,ticketnum);
            }

            //Ahora mismo todo el trabajo por parte del proceso está finalizado, se han enviado las peticiones y se encarga el receptor
            //La sincronización con el proceso se hace mediante un semaforo
            printf("Espero a ACK por los nodos...\n");
            fflush(stdout);
            sem_wait(&esperaRespuesta);
            printf("\n[Nodo %i]: He entrado en la sección crítica %i veces. Con el ticket: %i\n",nodos[0],contadorsc,ticketnum);
            sleep(rand() % 10 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            contadorsc++;
            // Recorrer la lista
            struct Nodo* actual = nodosenespera;
            while (actual != NULL) {
                printf("Notificando al nodo que ahora si puede entrar: %d \n", actual->valor);
            
                NetworkSend(red,nodos[0], actual->valor, NO_INTERESADO,pid, ACK, ticketnum);
                actual = actual->siguiente;
            }
            estado = 0;
            borrarLista(&nodosenespera);

            if (lastticket < ticketnum) lastticket=ticketnum;

    }while (1); // Bucle para que funcione constantemente*/ 
    return;
}
void reservas(){
    pid_t pid = getpid();
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
            printf("Va a entrar en la SC...\n");
            fflush(stdout);

            //Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
            //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
            ticketnum = lastticket + rand() % NODOSVECINOS + 5;
            

            
            //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

            for (int i=1;i<NODOSVECINOS; i++){
                fflush(stdout);
                NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,pid,SOLICITUD,ticketnum);
            }

            //Ahora mismo todo el trabajo por parte del proceso está finalizado, se han enviado las peticiones y se encarga el receptor
            //La sincronización con el proceso se hace mediante un semaforo
            printf("Espero a ACK por los nodos...\n");
            fflush(stdout);
            sem_wait(&esperaRespuesta);
            printf("\n[Nodo %i]: He entrado en la sección crítica %i veces. Con el ticket: %i\n",nodos[0],contadorsc,ticketnum);
            sleep(rand() % 10 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            contadorsc++;


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



            // Recorrer la lista
            struct Nodo* actual = nodosenespera;
            while (actual != NULL) {
                printf("Notificando al nodo que ahora si puede entrar: %d \n", actual->valor);
            
                NetworkSend(red,nodos[0], actual->valor, NO_INTERESADO,pid, ACK, ticketnum);
                actual = actual->siguiente;
            }
            estado = 0;
            borrarLista(&nodosenespera);

            if (lastticket < ticketnum) lastticket=ticketnum;

    }while (1); // Bucle para que funcione constantemente*/ 
    return;
}



void initparam(int argc, char *argv[]){
    //Parametros con las ID
    NODOSVECINOS=argc-4;
    printf("Se van a iniciar %i nodos",NODOSVECINOS);

    //Parametros a entero
    for (int i = 4; i < argc; i++) {
        nodos[i-4] = atoi(argv[i]);
    }

    srand(time(NULL)); //Para que los aleatorios no sean siempre los mismos!

    printf("Nodos vecinos son:\n");
    for (int i=0; i<NODOSVECINOS; i++) {
        printf("ID de nodo %i: %i    \n",i,nodos[i]);
    }

    //INICIALIZACIÓN DE SEMAFOROS
    sem_init(&sem_procesos,0,0);
    sem_init(&sem_solicitaSC,0,0);
    sem_init(&sem_crear_hilos,0,1);
    sem_init(&sem_consultas,0,0);
    sem_init(&sem_saleSC,0,0);

    //Compuebo que hay un receptor escuchando en los buzones correspondientes
    char command[256];
    snprintf(command, 256, "ps ax | grep -v grep | grep \"./multiproceso.o RECEP %s %s\" > /dev/null", argv[2], argv[3]);    if (system(command) == 0) {
        printf("Existe un receptor activo para este proceso.\n");
        return;
    }else{
        printf("NO Existe un receptor activo para este proceso.\n");
        sigint_handler(1);//TODO: Poner el numero del ctrl+c
    }

}