#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MSG_SIZE 128

struct config {
    int interval;
    char message[MSG_SIZE];
};

int load_config(const char *path, struct config *out)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;

    char line[256];

    while (fgets(line, sizeof(line), f)) {
        /* remove newline */
        line[strcspn(line, "\n")] = 0;

        /* skip empty lines */
        if (line[0] == '\0')
            continue;

        /* skip comments */
        if (line[0] == '#')
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

        } else if (strcmp(key, "message") == 0) {
            snprintf(out->message, MSG_SIZE, "%s", value);

        } else {
            /* unknown key */
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}
