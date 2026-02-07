#include <libudev.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void handle_sigterm(int sig) {
    (void)sig;
    running = 0;
}

void notify(const char *title, const char *body) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {

        char *argv[] = {"notify-send", (char *)title, (char *)body, NULL};

        execvp("notify-send", argv);

        perror("execvp");
        _exit(1);
    }
}

void check_action(const char *action) {
    if (action) {
        if (strcmp(action, "add") == 0) {
            printf("USB connected\n");
            notify("USB CONNECTED", "You connected a usb device");
        } else if (strcmp(action, "remove") == 0) {
            printf("USB removed\n");
            notify("USB DISCONNECTED", "You disconnected a usb device");
        }

        fflush(stdout);
    }
}

int main(void) {
    struct udev *udev;
    struct udev_monitor *mon;
    int fd;

    signal(SIGTERM, handle_sigterm);

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "cannot create udev context\n");
        return 1;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        fprintf(stderr, "cannot create udev monitor\n");
        return 1;
    }

    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_enable_receiving(mon);

    fd = udev_monitor_get_fd(mon);

    printf("usb event daemon started\n");
    fflush(stdout);

    while (running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        if (select(fd + 1, &fds, NULL, NULL, NULL) <= 0)
            continue;

        if (FD_ISSET(fd, &fds)) {
            struct udev_device *dev;

            dev = udev_monitor_receive_device(mon);
            if (!dev)
                continue;

            const char *action = udev_device_get_action(dev);

            check_action(action);

            udev_device_unref(dev);
        }
    }

    printf("usb event daemon shutting down\n");
    fflush(stdout);

    udev_monitor_unref(mon);
    udev_unref(udev);

    return 0;
}