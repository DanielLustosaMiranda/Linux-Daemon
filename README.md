# Minimal Daemon with systemd - Step-by-Step Demonstration

This document consolidates the full workflow for creating, supervising, and logging a minimal daemon using systemd.

---

## 1. Compile the daemon

Assuming your daemon source is `daemon.c`:

```bash
gcc -o daemon-min daemon.c
chmod +x daemon-min  # ensure executable
```

---

## 2. Create the systemd service

Create the folder if it doesn't exist:

```bash
mkdir -p ~/.config/systemd/user
```

Create the file `~/.config/systemd/user/daemon-min.service`:

```ini
[Unit]
Description=My minimal daemon
After=network.target

[Service]
Type=simple
ExecStart=/full/path/to/daemon-min
Restart=on-failure
RestartSec=5

[Install]
WantedBy=default.target
```

> Make sure `ExecStart` is the absolute path to your compiled binary.

---

## 3. Reload systemd to recognize the new service

```bash
systemctl --user daemon-reload
```

---

## 4. Start the daemon

```bash
systemctl --user start daemon-min.service
```

Your daemon will now run under systemd supervision.

---

## 5. View logs in real-time

```bash
journalctl --user -u daemon-min.service -f
```

You should see output from your daemon's `printf` statements like:

````
daemon iniciado
loop ativo
loop ativo
...```

---

## 6. Stop the daemon (SIGTERM test)

```bash
systemctl --user stop daemon-min.service
````

systemd sends `SIGTERM`, your handler sets `running = 0`, and the daemon exits cleanly.

Expected log:

```
recebi SIGTERM, encerrando...
```

---

## 7. Check service status

```bash
systemctl --user status daemon-min.service
```

* Shows PID, start/stop timestamps
* Exit code 0 indicates graceful termination

---

## Key Observations

1. **Compilation → binary ready**
2. **Service definition → supervision by systemd**
3. **stdout → logs captured by journal**
4. **Signals → graceful shutdown**
5. **Restart policy → automatic recovery on failure**

This demonstrates the complete minimal daemon lifecycle under systemd supervision.
