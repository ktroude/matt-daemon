# MattDaemon: A Minimal UNIX Daemon Server

## Overview

**MattDaemon** is a UNIX-compliant daemon written in C++ that listens on port `4242` and handles TCP connections. Built to emulate real-world daemon behavior, the project features:

- Root-only execution  
- Single-instance enforcement using lock files  
- Concurrent client handling (up to 3 clients)  
- Graceful shutdown via signal handling and command parsing  
- Detailed logging using a custom `TintinReporter` class  

This project demonstrates process management, socket programming, and secure daemon design in a UNIX environment.

## Features

- Daemonization: Runs as a proper background process detached from the terminal.
- Single Instance: Ensures only one daemon is running using `/var/lock/matt_daemon.lock`.
- TCP Socket Server: Listens on port `4242` and accepts up to 3 simultaneous clients.
- Logging: Uses `TintinReporter` to log events with timestamps in `/var/log/matt_daemon/matt_daemon.log`.
- Signal Handling: Handles `SIGINT` and `SIGTERM` to shut down cleanly.
- Controlled Shutdown: The server shuts down on receiving the string `quit` from a client.
- Error Reporting: Errors are logged with contextual messages and timestamps.

## Technologies

- Language: C++17
- Socket Programming: POSIX sockets
- Process Handling: `fork`, `setsid`, `umask`, `signal`
- File Locking: `flock`-based mutual exclusion
- Logging: Thread-safe logger with timestamped entries

## Installation and Usage

### Prerequisites

- UNIX-like system (Linux recommended)
- Root privileges
- `make`, `g++`

### Build

```bash
make
```

### Run (as root)

```bash
sudo ./MattDaemon
```

### Connect using netcat

```bash
nc localhost 4242
```

## Example

```bash
# Run daemon

sudo ./MattDaemon

# Connect with client
nc localhost 4242
Hello
quit

# View logs
cat /var/log/matt_daemon/matt_daemon.log
[17/06/2025 - 15:56:44] [INFO] - Matt_daemon: Server created.
[17/06/2025 - 15:56:44] [INFO] - Matt_daemon: Entering Daemon mode.
[17/06/2025 - 15:56:56] [INFO] - Matt_daemon: User 6 connected
[17/06/2025 - 15:57:01] [LOG] - Matt_daemon: User 6 input: Hello
[17/06/2025 - 15:58:06] [INFO] - Matt_daemon: Request quit.
[17/06/2025 - 15:58:06] [INFO] - Matt_daemon: Client handler thread exited.
[17/06/2025 - 15:58:06] [INFO] - Matt_daemon: Lock file removed.
```

## Makefile Targets

- make – Build the executable
- make clean – Remove object files
- make fclean – Remove object files and executable
- make re – Rebuild everything from scratch

## Development Notes

- The daemon quits properly on signal reception (SIGINT, SIGTERM).
- The lock file is cleaned up after exit.
- Server handles a maximum of 3 concurrent clients.
- Logging is thread-safe and includes timestamps.
