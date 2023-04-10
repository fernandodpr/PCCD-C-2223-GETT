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
    // Tendríamos que hacer una función de inicialización de los buzones

   
    //Aleatorizar la entrada en SC
    double random_value;
    bool   quiero_entrar;
    do {
        random_value = (double)rand() / RAND_MAX;
        quiero_entrar = random_value > PROBABILIDAD_ENTRADA;
        if (!quiero_entrar) {
           sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos
        }else{}
    }while (!quiero_entrar);

    //Ahora el proceso ya quiere entrar en SC
    bool acceso=false;

    //Comprobar si hay otro poceso que nos notificó su deseo de entrar



    // Sección crítica
    if(acceso){
        printf("[Nodo %i]: He entrado en la sección crítica",identificador_nodo);
        sleep(rand() % 5 + 4); // Dormir una cantidad de tiempo aleatoria entre 4 y 8 segundos    }
        printf("[Nodo %i]: He terminado la sección crítica",identificador_nodo);
    }
    //Salgo de la sección critica:
        //Cambio mi estado a que no quiero entrar
        quiero_entrar = false;
        // Cambio mi estado a que ya no
        estado=0;
        //Notifico a los nodos que ya salí de SC
        //De momento está en pseudocodigo
        for(i=0;i<cantidaddenodos;i++){
            //Enviar a cada nodo un mensaje de que ya salió de SC
           
            printf("[Nodo %i]: Mensaje enviado a nodo %i. Contenido: Ya no quiero entrar en SC\n",identificador_nodo,nodovecino);
        }

    return 0;
}