#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PROBABILIDAD_ENTRADA 0.5
#define PROBABILIDAD_PERDIDA_PAQUETE 0.01



typedef enum {
    NO_INTERESADO, //0  El proceso no está interesado en acceder a la sección crítica actualmente
    SOLICITANTE, //1  El proceso está solicitando acceso a la sección crítica
    ESPERANDO   //2  El proceso está esperando su turno para acceder a la sección crítica
} Status;


typedef struct {
    int id_nodo;        //Origen del paquete
    int id_proceso;     //Origen peticion PID
    int num_ticket ;    //El número de ticket  
    Status estado;      //En que estado se encuentra
} Paquete;

// Función dummy para enviar un mensaje a otro proceso
void NetworkSend(int destination, char* message) {
    // TODO: Implementar la lógica de envío de mensaje a otro proceso
}

// Función dummy para recibir un mensaje de otro proceso
char* networkrcv() {
    // TODO: Implementar la lógica de recepción de mensajes de otros procesos
    char* message = NULL; // Aquí se debe almacenar el mensaje recibido
    return message;
}

//Memoria y variables del proceso
int identificador_nodo;
Status estado = 0;


void* recepcion(void* args){

}
int main(int argc, char *argv[]) {



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



        //Ahora el proceso ya quiere entrar en SC
        bool acceso=false;
        //Comprobar si hay otro poceso que nos notificó su deseo de entrar
        //Aqui, en función de esto es donde se asigna el valor  a estado.


//EL RECEPTOR: ME LLEGA SOLICITD, SI NADIE EN MI NODO QUISO ENTRAR LE DOY PERMISO- DUDA: NOS INTERESA PONER AQUI EL ESTADO COMO ESPERANDO??


// Si el estado es esperando significa que un nodo entro y estamos esperando a que termine su SC. ¿Con un semaforo de condicion? do while sigaamos esp


        //SI NO ESTAMOS ESPERANDO....
//Bloquear la variable estado
// estado solicitante
        for(i=0;i<cantidaddenodos;i++){
                //Enviar a cada nodo un mensaje de que quiero entrar en SC
                printf("[Nodo %i]: Mensaje enviado a nodo %i. Contenido: Ya no quiero entrar en SC\n",identificador_nodo,nodovecino);
            }

//RECIBIR UN MENSAJE POR NODO CON OK




        // Sección crítica
        if(acceso){
            printf("[Nodo %i]: He entrado en la sección crítica",identificador_nodo);
            sleep(rand() % 5 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
            printf("[Nodo %i]: He terminado la sección crítica",identificador_nodo);
        }


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

    }while (1); // Bucle para que funcione constantemente
    return 0;
}