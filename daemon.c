#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MSG_SIZE 128
#define CONFIG_PATH "/home/lloyd101/system_monitor/daemon/daemon.conf"

// Configuration
struct config {
    int interval;
    char message[MSG_SIZE];
};

void config_set_defaults(struct config *config) {
    config->interval = 5;
    snprintf(config->message, MSG_SIZE, "hello");
}

int load_config(const char *path, struct config *out) {
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;

    char line[256];

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;

        if (line[0] == '\0' || line[0] == '#')
            continue;

        char *eq = strchr(line, '=');
        if (!eq) {
            fclose(f);
            return -1;
        }

        *eq = '\0';
        const char *key = line;
        const char *value = eq + 1;

        if (strcmp(key, "interval") == 0) {
            char *end;
            long v = strtol(value, &end, 10);
            if (*end != '\0' || v <= 0) {
                fclose(f);
                return -1;
            }
            out->interval = (int)v;
        }
        else if (strcmp(key, "message") == 0) {
            snprintf(out->message, MSG_SIZE, "%s", value);
        }
        else {
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

// Daemon state
typedef enum {
    MODE_NORMAL = 0,
    MODE_DEBUG  = 1
} daemon_mode_t;

volatile sig_atomic_t running = 1;
volatile sig_atomic_t reload_requested = 0;
volatile sig_atomic_t switch_to_debug  = 0;
volatile sig_atomic_t switch_to_normal = 0;

volatile sig_atomic_t current_mode = MODE_NORMAL;

// Signal handlers
void handle_sigterm(int sig) {
    (void)sig;
    running = 0;
}

void handle_sighup(int sig) {
    (void)sig;
    reload_requested = 1;
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
    signal(SIGHUP,  handle_sighup);
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);

    struct config cfg;
    config_set_defaults(&cfg);

    if (load_config(CONFIG_PATH, &cfg) == 0) {
        printf("config loaded at startup: interval=%d message='%s'\n",
               cfg.interval, cfg.message);
    } else {
        printf("using default config\n");
    }

    fflush(stdout);

    time_t next_tick = time(NULL) + cfg.interval;

    printf("daemon started in NORMAL mode\n");
    fflush(stdout);

    while (running) {
        time_t now = time(NULL);

        /* mode switching */
        if (switch_to_debug) {
            current_mode = MODE_DEBUG;
            printf("mode switched to DEBUG\n");
            fflush(stdout);
            switch_to_debug = 0;
        }

        if (switch_to_normal) {
            current_mode = MODE_NORMAL;
            printf("mode switched to NORMAL\n");
            fflush(stdout);
            switch_to_normal = 0;
        }

        /* config reload */
        if (reload_requested) {
            reload_requested = 0;

            struct config new_cfg;
            config_set_defaults(&new_cfg);

            if (load_config(CONFIG_PATH, &new_cfg) == 0) {
                cfg = new_cfg;
                printf("config reloaded: interval=%d message='%s'\n",
                       cfg.interval, cfg.message);
                next_tick = now + cfg.interval;
            } else {
                printf("config reload failed, keeping old config\n");
            }

            fflush(stdout);
        }

        /* periodic behavior */
        if (now >= next_tick) {
            if (current_mode == MODE_NORMAL) {
                printf("[normal] %s\n", cfg.message);
            } else {
                printf("[debug] %s — extra diagnostics\n", cfg.message);
            }

            fflush(stdout);
            next_tick += cfg.interval;
        }

        usleep(200000); /* responsive, CPU-friendly */
    }

    printf("daemon shutting down\n");
    fflush(stdout);
    return 0;
}