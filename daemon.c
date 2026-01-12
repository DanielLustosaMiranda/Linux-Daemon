#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handle_sigterm(int sig) {
    (void)sig;     // evita warning
    running = 0;
}

int main() {
    signal(SIGTERM, handle_sigterm);

    printf("daemon temporizador iniciado\n");
    fflush(stdout);
    
    const int interval = 5; // segundos
    while (running) {
        // ação mínima: log com timestamp
        time_t now = time(NULL);
        printf("tick at %s", ctime(&now));
        fflush(stdout);
        // dormir de forma eficiente
        sleep(interval);
    }

    printf("daemon temporizador encerrado\n");
    fflush(stdout);

    return 0;
}
