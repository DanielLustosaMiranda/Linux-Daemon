#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
 
typedef enum{
    MODE_NORMAL = 0,
    MODE_DEBUG = 1
} daemon_mode_t;

volatile sig_atomic_t current_mode = MODE_NORMAL;
volatile sig_atomic_t switch_to_debug  = 0;
volatile sig_atomic_t switch_to_normal = 0;
volatile sig_atomic_t running = 1;

void handle_sigterm(int sig) {
    (void)sig;
    running = 0;
}

void handle_sigusr1(int sig) {
    (void)sig;
    switch_to_debug = 1;
}

void handle_sigusr2(int sig) {
    (void)sig;
    switch_to_normal = 1;
}

int main() {
    signal(SIGTERM, handle_sigterm);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    printf("daemon started in NORMAL mode\n");
    fflush(stdout);

    const int interval = 5;
    time_t next_tick = time(NULL) + interval;

    while (running) {
        time_t now = time(NULL);

        if (switch_to_debug) {
            current_mode = MODE_DEBUG;
            printf("Modes switched to debug\n");
            fflush(stdout);
            switch_to_debug = 0;
        }

        if (switch_to_normal) {
            current_mode = MODE_NORMAL;
            printf("Mode switched to normal\n");
            fflush(stdout);
            switch_to_normal = 0;
        }

        /* periodic behavior */
        if (now >= next_tick) {
            if (current_mode == MODE_NORMAL) {
                printf("mode: normal\n");
            } else {
                printf("mode: debug — extra diagnostics\n");
            }
            fflush(stdout);
            next_tick += interval;

            usleep(500000); // 0.5s — responsive, CPU-friendly
        }
    }

    printf("daemon shutting down\n");
    fflush(stdout);
    return 0;
}