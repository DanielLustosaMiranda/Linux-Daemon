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

    printf("responsive daemon started\n");
    fflush(stdout);

    const int interval = 5; // seconds
    time_t next_tick = time(NULL) + interval;

    while(running){
        time_t now = time(NULL);

        if(now >= next_tick){
            printf("tick at %s", ctime(&now));
            fflush(stdout);
            next_tick += interval;
        }

        usleep(500000); // 500ms
    }

    printf("deamon responsive sttoped\n");
    fflush(stdout);
    return 0;
}