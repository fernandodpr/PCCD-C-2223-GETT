#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
pthread_mutex_t mutex;

void* imprimir_pid_ppid(void* arg) {
    pthread_mutex_lock(&mutex);
    pid_t pid = gettid();
    pid_t ppid = getppid();
    fflush(stdout);
    printf("Soy el hilo con PID %d y mi PPID es %d\n", pid, ppid);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[5];
    
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < 5; i++) {
        pthread_create(&hilos[i], NULL, imprimir_pid_ppid, NULL);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(hilos[i], NULL);
    }

    return 0;
}
