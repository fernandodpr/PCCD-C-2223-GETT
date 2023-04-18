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

sem_t sem_consultas;
sem_t sem_SC;
sem_t sem_saleSC;
sem_t sem_espera_ACK;
sem_t sem_avisar_nodos_en_espera;

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
    printf("multiproceso.o ID_COLA_RED\n\n");
    printf("multiproceso.o ID_COLA_RED ID_NODO [ID'S NODOS]\n");
    printf("  ####TIPO: El tipo de proceso a lanzar: consultas,reservas...\n");
    printf("  #####ID_COLA_INTERNA:  Identificador del buzón interno de sincronizacion.\n");
    printf("  ID_COLA_RED:      Identificador de la cola sobre la que se simula la red\n");
    printf("  ID_NODO:          Identificador del nodo donde se ejecuta\n");
    printf("  [ID'S NODOS]:     Identificadores de nodos vecinos\n");
    // Agrega más información de ayuda según sea necesario
}
int main(int argc, char *argv[]) {

    //Parámetros
    if (argc < 3 || (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        // Muestra la ayuda y sale del programa
        ayuda();
        return 0;
    }

    //Signal de salida
    printf("Presione Ctrl+C para salir del programa.\n");
    signal(SIGINT, sigint_handler);


    
    //EJECUTO EL RECEPTOR
    //Creación del buzón del nodo
    int proj_id=52; //TODO: Esto hay que parametrizarlo
    key_t key=ftok("/bin/ls",proj_id);
    red=msgget(99999, IPC_CREAT | 0777);
    //printf("red establecida: %i\n",red);

    initparam(argc, argv);

    pthread_t pthrecepcion;
    pthread_create(&pthrecepcion,NULL,(void *)recepcion,NULL);   
    pthread_t pthtest[10];
    for (int i =0; i<10; i++) {
        printf("Creo hilo");
        pthread_create(&pthtest[i],NULL,(void *)procesomutex,NULL);   
    }
    pthread_t pthtest2[10];
    for (int i =0; i<10; i++) {
        printf("Creo hilo");
        pthread_create(&pthtest2[i],NULL,(void *)procesoconsultas,NULL);   
    }
    
    while (1) {
    
    }
    return 0;
}

// CREA numProcesos HILOS de PROCESOS con tipo typeProceso
// typeProceso = 5 -> CONSULTAS
// typeProceso = 4 -> DEMÁS
int creaHiloProceso(int typeProceso, int numProcesos){

    pthread_t procesoHilos[1000];

    for(int i = 0; i < numProcesos; i++){
        pthread_create(&procesoHilos[i],NULL,(void *)procesoHilo,(void *)&typeProceso);
        colaConsultas++;
        sem_wait(&sem_crear_hilos);
    }

    return 0;

}


void * procesoHilo(int * param){
/*
    // TODO: por ahora solo consultas. Despues coge el param para crear otro tipo de procesos
    int procesotype = 5;
    //int *pcolaConsultas = &colaConsultas;
    sem_t* sem_proceso;
    //Si el proceso no es una consulta, se desactiva las consultas para poder escribir.
    if(procesotype != 5){
        consultasActivas = 0;
    }
    sem_proceso = &sem_consultas;

    printf("Creado proceso de tipo: %i\n", procesotype);
    sem_post(&sem_crear_hilos);
    sleep(5);

    if(procesotype != 5){
        sem_post(&sem_solicitaSC);
    }

    if(procesotype == 5 && consultasActivas == 1 && consultasEnSC == 1){
        //El proceso creado es CONSULTA, las consultas están ACTIVAS y hay consultas en SC, continua a la SC
    }else{
        //Esperamos a que el nodo nos de permiso
        sem_post(&sem_solicitaSC);
        sem_wait(sem_proceso);
    }
    printf("PROCESO con tipo: %i entra a la SC\n", procesotype);
    if(procesotype == 5){
        consultasActivas = 1;
        consultasEnSC = 1;
    }
    if(procesotype == 5){
        sem_post(&sem_solicitaSC);
    }
    sleep(7);
    printf("PROCESO con tipo:%i sale de la SC\n", procesotype);
    sem_post(&sem_saleSC);
    consultasEnSC = 0;
    printf("FIN PROCESO type: %i", procesotype);
    colaConsultas--;
    pthread_exit(NULL);
*/
}



void * procesomutex(int * param){

    pid_t pid = getpid();
        // Tendríamos que hacer una función de inicialización de los buzones
        // Es necesario meter semaforos sobre la variable comptartida entre receptor y main (estado y ticket)
        // Contienda entre los procesos del mismo nodo. No sabes a que proceso envia. ESO SERIA FACIL SI SE ENVIA EL PROCESO EN EL MENSAJE ID PROCESO EN LAS PETICIONES Y EN LAS RESPUESTAS. Asi sabe para quien es.
        //UN PROCESO NO SABE SI LOS QUE QUIEREN ENTRAR ESTAN EN SU MISMO NODO O NO
        // EL RECEPTOR CONTESTA A LOS MENSAJES
        // COMUNICAR RECEPTOR CON PROCESOS EN EL MISMO NODO?
        // 
    do{

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

        //Comprobar si alguien en el nodo está en SC


        //Manda solicitudes
            //Espera en Semáforo por respuestas

            //Cuando tiene respuestas: Entra en SC

        
            //Envio de solicitudes

            bool necesariasolicitud=false;
            if(contarNodos(nodosenespera)>0){
                //Hay más nodos en espera no puedo entrar en SC sin solicitud
                necesariasolicitud=true;
            }else{
                //Ningún nodo ha solicitado acceso, puedo seguir en SC
                necesariasolicitud=false;

            }
            
            printf("\nVoy a esperar %lu\n",pthread_self());

            //Esperar en el semáforo de las consultas
            sem_wait(&sem_SC);
            printf("Nadie en el nodo tiene SC.");
            

           
            if(necesariasolicitud){
                printf("Notificar a los nodos de SC");

                /*//Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
                //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
                ticketnum = lastticket + rand() % NODOSVECINOS + 5;
                //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

                for (int i=1;i<NODOSVECINOS; i++){
                    NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,pid,SOLICITUD,ticketnum);
                }*/

                //sem_wait(&sem_espera_ACK); //Nos manda el permiso el receptor
                printf("Recibidos todos los ACK, entro en SC");

            }

            //SECCION CRITICA
            printf("\n[Hilo %lu]: He entrado en la sección crítica %i veces. Con el ticket: %i\n",pthread_self(),contadorsc,ticketnum);
            sleep(rand() % 10 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            contadorsc++;
            


            /*
                //Cómo detectar que es el último proceso de los que no pidieron solicitud
                //Para enviar los aCK
                int valorSemaforo;
                sem_getvalue(&sem_SC, &valorSemaforo);

                if(valorSemaforo==0){
                    //Hay procesos en el nodo esperando la SC

                //Les doy paso a SC
                sem_post(&sem_SC);
                sem_wait(&sem_avisar_nodos_en_espera);
                //Espero en semáforo para avisar al salir 
                }else{
                    //No hay procesos en espera
                    sem_post(&sem_avisar_nodos_en_espera); 
                }

                int valorSemaforo;
                sem_getvalue(&semaforo, &valorSemaforo);

                */

                /*
                //Notificacion de nodos en espera
                // Recorrer la lista
                struct Nodo* actual = nodosenespera;
                while (actual != NULL) {
                    printf("Notificando al nodo que ahora si puede entrar: %d \n", actual->valor);
                
                    NetworkSend(red,nodos[0], actual->valor, NO_INTERESADO,pid, ACK, ticketnum);
                    actual = actual->siguiente;
                }
                estado = 0;
                borrarLista(&nodosenespera);

                if (lastticket < ticketnum) lastticket=ticketnum;*/


            sem_post(&sem_SC);
            printf("Despierta");


    }while(1);
    
}

void * procesoconsultas(int * param){

    pid_t pid = getpid();
        // Tendríamos que hacer una función de inicialización de los buzones
        // Es necesario meter semaforos sobre la variable comptartida entre receptor y main (estado y ticket)
        // Contienda entre los procesos del mismo nodo. No sabes a que proceso envia. ESO SERIA FACIL SI SE ENVIA EL PROCESO EN EL MENSAJE ID PROCESO EN LAS PETICIONES Y EN LAS RESPUESTAS. Asi sabe para quien es.
        //UN PROCESO NO SABE SI LOS QUE QUIEREN ENTRAR ESTAN EN SU MISMO NODO O NO
        // EL RECEPTOR CONTESTA A LOS MENSAJES
        // COMUNICAR RECEPTOR CON PROCESOS EN EL MISMO NODO?
        // 
    do{

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

        //Comprobar si alguien en el nodo está en SC


        //Manda solicitudes
            //Espera en Semáforo por respuestas

            //Cuando tiene respuestas: Entra en SC

        
            //Envio de solicitudes

            bool necesariasolicitud=false;
            if(contarNodos(nodosenespera)>0){
                //Hay más nodos en espera no puedo entrar en SC sin solicitud
                necesariasolicitud=true;
            }else{
                //Ningún nodo ha solicitado acceso, puedo seguir en SC
                necesariasolicitud=false;

            }
            int valorSemaforoSC;
            sem_getvalue(&sem_SC, &valorSemaforoSC);
            int valorSemaforoConsultas;
            sem_getvalue(&sem_consultas, &valorSemaforoConsultas);
            sem_wait(&sem_SC);
            printf("Una consulta entra en SC");
            sem_wait(&sem_consultas);

            /*if((valorSemaforoConsultas<5)&&(necesariasolicitud == false)){
                //Hay consultas esperando entre si o hay consultas en SC
                printf("\nSOY consulta Espero en Consultas %lu\n",pthread_self());
                sem_wait(&sem_consultas);
            }else{
                printf("\nSOY consulta Espero en SC %lu\n",pthread_self());
                sem_wait(&sem_SC);

            }*/
            
            printf("\nSOY consulta Voy a esperar %lu\n",pthread_self());

            //Esperar en el semáforo de las consultas
            sem_wait(&sem_SC);
            printf("SOY consulta Nadie en el nodo tiene SC.");
            

           
            if(necesariasolicitud){
                printf("SOY consulta Notificar a los nodos de SC");

                /*//Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
                //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
                ticketnum = lastticket + rand() % NODOSVECINOS + 5;
                //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

                for (int i=1;i<NODOSVECINOS; i++){
                    NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,pid,SOLICITUD,ticketnum);
                }*/

                //sem_wait(&sem_espera_ACK); //Nos manda el permiso el receptor
                printf("SOY consulta Recibidos todos los ACK, entro en SC");

            }

            //SECCION CRITICA
            printf("\n[Hilo %lu]: SOY consulta He entrado en la sección crítica %i veces. Con el ticket: %i\n",pthread_self(),contadorsc,ticketnum);
            sleep(rand() % 10 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            contadorsc++;
            


            /*
                //Cómo detectar que es el último proceso de los que no pidieron solicitud
                //Para enviar los aCK
                int valorSemaforo;
                sem_getvalue(&sem_SC, &valorSemaforo);

                if(valorSemaforo==0){
                    //Hay procesos en el nodo esperando la SC

                //Les doy paso a SC
                sem_post(&sem_SC);
                sem_wait(&sem_avisar_nodos_en_espera);
                //Espero en semáforo para avisar al salir 
                }else{
                    //No hay procesos en espera
                    sem_post(&sem_avisar_nodos_en_espera); 
                }

                int valorSemaforo;
                sem_getvalue(&semaforo, &valorSemaforo);

                */

                /*
                //Notificacion de nodos en espera
                // Recorrer la lista
                struct Nodo* actual = nodosenespera;
                while (actual != NULL) {
                    printf("Notificando al nodo que ahora si puede entrar: %d \n", actual->valor);
                
                    NetworkSend(red,nodos[0], actual->valor, NO_INTERESADO,pid, ACK, ticketnum);
                    actual = actual->siguiente;
                }
                estado = 0;
                borrarLista(&nodosenespera);

                if (lastticket < ticketnum) lastticket=ticketnum;*/


            sem_post(&sem_SC);
            sem_post(&sem_consultas);
            printf("SOY consulta Despierta");


    }while(1);
    
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

            //Antes de mandar la petición tengo que generar mi numero de ticket: Esto es el numero de ticket mas grande conocido+1
            //Duda: Puede dar pie a contienda mas comun el incrementar de uno en uno?
            ticketnum = lastticket + rand() % NODOSVECINOS + 5;
            

            
            //NOTIFICACION A NODOS VECINOS Duda: que pasa si me notifico a mi mismo

            for (int i=1;i<NODOSVECINOS; i++){
                NetworkSend(red,nodos[0],nodos[i],SOLICITANTE,pid,SOLICITUD,ticketnum);
            }

            //Ahora mismo todo el trabajo por parte del proceso está finalizado, se han enviado las peticiones y se encarga el receptor
            //La sincronización con el proceso se hace mediante un semaforo
            printf("Espero a ACK por los nodos...\n");
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
    sem_init(&sem_SC,0,1);
    sem_init(&sem_espera_ACK,0,1);
    sem_init(&sem_consultas,0,5);
    sem_init(&sem_saleSC,0,0);
    sem_init(&sem_avisar_nodos_en_espera,0,0);
    

    //creaHiloProceso(5,3);



}