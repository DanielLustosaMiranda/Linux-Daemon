#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handle_sigterm(int sig) {
    (void)sig;     // evita warning
    running = 0;
}

int main() {
    signal(SIGTERM, handle_sigterm);

    printf("daemon iniciado\n");
    fflush(stdout);

    while (running) {
        printf("loop ativo\n");
        fflush(stdout);
        sleep(5);
    }

    printf("recebi SIGTERM, encerrando...\n");
    fflush(stdout);

    return 0;
}
