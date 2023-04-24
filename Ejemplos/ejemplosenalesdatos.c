#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void sigusr1_handler(int sig, siginfo_t *si, void *unused) {
    printf("Señal SIGUSR1 recibida con valor %d\n", si->si_value.sival_int);
    // Aquí puedes procesar los datos recibidos
}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Error al instalar manejador de señal");
        return 1;
    }
    printf("Esperando la señal SIGUSR1...\n");
    // Tu programa seguirá ejecutándose y esperando la señal SIGUSR1
    while (1) {
        // Hacer algo útil
    }
    return 0;
}

void enviar_signal_con_datos(pid_t pid, int valor) {
    union sigval sv;
    sv.sival_int = valor;
    if (sigqueue(pid, SIGUSR1, sv) == -1) {
        perror("Error al enviar la señal");
        exit(EXIT_FAILURE);
    }
}