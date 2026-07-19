# usb-notifier — USB Event Daemon

A lightweight Linux daemon that monitors USB connect/disconnect events via **libudev** and sends desktop notifications using `notify-send`. Managed as a **systemd user service**.

---

## Project Structure

```
usb_daemon/
├── src/
│   ├── daemon.c        # Main daemon logic (udev monitor + notifications)
│   ├── loadconfig.c    # Config file parser
│   └── daemon.conf     # Example configuration file
├── assets/
│   └── usb-icon.jpeg   # Icon used in desktop notifications
├── usb-notifier.service  # systemd user service definition
├── CMakeLists.txt
├── install.sh
└── uninstall.sh
```

---

## Dependencies

| Dependency     | Purpose                          |
|----------------|----------------------------------|
| `libudev`      | Kernel USB event monitoring      |
| `notify-send`  | Desktop notification dispatch    |
| `cmake` ≥ 3.12 | Build system                     |
| `pkg-config`   | Library discovery                |

Install on Debian/Ubuntu:
```bash
sudo apt install libudev-dev libnotify-bin cmake pkg-config
```

---

## Build

```bash
cmake -B build
cmake --build build
```

The compiled binary will be at `build/usb-notifier`.

---

## Install & Uninstall

### Install
```bash
./install.sh
```

What it does:
1. Copies the icon to `/usr/share/usb-notifier/`
2. Copies the binary to `/usr/bin/usb-notifier`
3. Copies the service file to `~/.config/systemd/user/`
4. Runs `systemctl --user daemon-reload`
5. Enables and starts the service

### Uninstall
```bash
./uninstall.sh
```

What it does:
1. Stops and disables the systemd user service
2. Removes the service file and reloads the daemon
3. Removes the binary from `/usr/bin/`
4. Removes the asset directory `/usr/share/usb-notifier/`

---

## Configuration

The config file is parsed by `loadconfig.c`. Format: `key=value`, one per line. Lines starting with `#` are treated as comments.

**Example** (`src/daemon.conf`):
```ini
# Poll interval in seconds
interval=5

# Custom message
message=hello
```

### Supported Keys

| Key        | Type   | Description                        |
|------------|--------|------------------------------------|
| `interval` | `int`  | Must be a positive integer (> 0)   |
| `message`  | `string` | Arbitrary text, max 128 chars    |

> Unknown keys cause `load_config()` to return `-1` (parse error).

---

## Function Reference

### `daemon.c`

#### `void handle_sigterm(int sig)`
Signal handler registered for `SIGTERM`. Sets the global `running` flag to `0`, causing the main loop to exit cleanly.

#### `void notify(const char* title, const char* body, const char* urgency, const char* icon_path)`
Forks a child process and calls `notify-send` via `execvp` to display a desktop notification. The parent returns immediately (fire-and-forget). Any `fork` or `execvp` errors are printed to `stderr`.

| Parameter    | Description                                      |
|--------------|--------------------------------------------------|
| `title`      | Notification title                               |
| `body`       | Notification body text                           |
| `urgency`    | Urgency level: `low`, `normal`, or `critical`   |
| `icon_path`  | Absolute path to the notification icon           |

#### `void check_action(const char* action)`
Inspects the udev action string and dispatches the appropriate notification:
- `"add"` → **"USB CONNECTED"**
- `"remove"` → **"USB DISCONNECTED"**

#### `int main(void)`
Entry point. Sets up the `SIGTERM` handler, initializes a **udev** context and monitor filtered to the `usb` subsystem, then enters a `select()`-based event loop. On each USB event, it calls `check_action()`. On `SIGTERM`, it cleans up and exits gracefully.

---

### `loadconfig.c`

#### `int load_config(const char* path, struct config* out)`
Parses a simple `key=value` config file into a `struct config`.

**Returns:** `0` on success, `-1` on any error (file not found, malformed line, unknown key, or invalid value).

**`struct config` fields:**

| Field      | Type          | Description                         |
|------------|---------------|-------------------------------------|
| `interval` | `int`         | Polling interval parsed from config |
| `message`  | `char[128]`   | Message string parsed from config   |

---

## Service Management

```bash
# Check service status
systemctl --user status usb-notifier

# View live logs
journalctl --user -u usb-notifier -f

# Restart the daemon
systemctl --user restart usb-notifier

# Stop the daemon
systemctl --user stop usb-notifier
```

---

## How It Works

```
libudev kernel events
        │
        ▼
  udev_monitor (usb subsystem filter)
        │
   select() loop
        │
   check_action()
        │
  ┌─────┴──────┐
  │            │
add          remove
  │            │
notify()    notify()
  │            │
fork + execvp("notify-send", ...)
```

1. The daemon subscribes to kernel USB events through a **udev netlink socket**.
2. `select()` blocks until an event arrives (no busy-waiting).
3. The action string (`"add"` / `"remove"`) is checked and the correct desktop notification is fired via a forked `notify-send` process.
4. On `SIGTERM` (e.g. `systemctl stop`), the loop exits and udev resources are freed cleanly.
