# Maxxwell Server Interface (MXSI)

## Overview

MXSI is the server management framework for the Maxxwell hosted
microkernel. It provides the infrastructure for user-space servers
to register, communicate, and be managed by the kernel.

## Architecture

```
+---------------------------------------------------------------+
|  Kernel                                                        |
|  +--------+  +-------+  +----------+  +--------+  +---------+ |
|  | Sched  |  | IPC   |  | Ports    |  | Tasks  |  | Server  | |
|  |        |  |       |  |          |  |        |  | Manager | |
|  +--------+  +-------+  +----------+  +--------+  +---------+ |
|                                                     |         |
|                   Kernel-mediated IPC                |         |
|                                                     v         |
|  +--------+  +--------+  +----------+  +----------+---------+ |
|  | Server |  | Server |  | Server   |  | Server   | MsWaiter| |
|  | 4bsd   |  | Device |  | NetServer|  | Audio    | Monitor | |
|  |        |  | Kit    |  |          |  | Server   |         | |
|  +--------+  +--------+  +----------+  +----------+---------+ |
|                                                               |
|  User-space Servers (isolated, IPC-only communication)        |
+---------------------------------------------------------------+
```

## Components

### server.h / server.c

Core server object definition and lifecycle management.

Server states:
- STOPPED  - Not loaded or initialized
- LOADING  - Executable being loaded
- STARTING - Server process starting
- READY    - Server running and accepting requests
- WAITING  - Server idle, waiting for events
- FAILED   - Server failed during start
- PANIC    - Server encountered unrecoverable error
- STOPPING - Server being shut down

### registry.h / registry.c

Server registry maintained by the kernel.

Lookup operations:
- By server ID
- By server name
- By capability
- By capability mask (any match)

### manager.h / manager.c

Server lifecycle manager.

Public API:
- mx_server_start(id) / mx_server_start_by_name(name)
- mx_server_stop(id) / mx_server_stop_by_name(name)
- mx_server_restart(id) / mx_server_restart_by_name(name)
- mx_server_list(buf, max)
- mx_server_find(query, out)

Commands (for MsWaiter integration):
- servers            - List all registered servers
- start <server>     - Start a server
- stop <server>      - Stop a server
- restart <server>   - Restart a server
- info <server>      - Show server details
- trace <server>     - Trace server activity
- panic <server>     - Mark server as panicked

### message.h / message.c

Kernel-mediated message passing between servers.

Message types:
- MX_MSG_SYNC    - Synchronous (sender blocks for reply)
- MX_MSG_ASYNC   - Asynchronous (fire-and-forget)
- MX_MSG_NOTIFY  - Notification (one-way event)
- MX_MSG_REPLY   - Reply to a previous message
- MX_MSG_REQUEST - Request
- MX_MSG_RESPONSE - Response

### ipc.h / ipc.c

IPC transport layer for server communication.

Uses Mach ports (Unix domain socket pairs) as the underlying
transport. Messages are validated by the kernel before delivery.

- mx_server_send()      - Send message to a server
- mx_server_reply()     - Reply to a request
- mx_server_broadcast() - Send to all servers with matching caps
- mx_server_notify()    - Send notification
- mx_server_rpc()       - Synchronous RPC call

### loader.h / loader.c

Server executable loader. Uses the kernel's mach_loader to load
.mach format binaries, creates tasks and threads, sets up IPC
endpoints, and waits for the server to signal READY.

### bootstrap.h / bootstrap.c

Bootstrap sequence:

1. Kernel initializes
2. IPC subsystem starts
3. Server manager initializes
4. Configured bootstrap server loads (from -srv= boot arg)
5. Server registers with kernel
6. Remaining servers may start
7. If configured server fails, MsWaiter starts automatically

## Capabilities

Servers advertise capabilities that clients can query:

- MX_CAP_FILESYSTEM - Filesystem services
- MX_CAP_DISPLAY    - Display/framebuffer
- MX_CAP_NETWORK    - Networking
- MX_CAP_AUDIO      - Audio
- MX_CAP_PROCESS    - Process management
- MX_CAP_BSD        - BSD compatibility
- MX_CAP_DEVICE     - Device access
- MX_CAP_STORAGE    - Storage
- MX_CAP_IPC        - IPC services
- MX_CAP_MONITOR    - System monitoring
- MX_CAP_INPUT      - Input handling
- MX_CAP_BOOTSTRAP  - Bootstrap server

## Example Server

See `example_server.c` for a complete server implementation that:
1. Registers with the kernel
2. Advertises a service port
3. Handles ping/info/shutdown messages
4. Sends READY signal to kernel
5. Enters dispatch loop

## Bootstrap Configuration

Boot argument format:
```
-srv=/path/to/bootstrap/server
```

If no server is configured, MsWaiter starts automatically as the
default bootstrap server.

## Future Compatibility

The interface supports:
- Remote servers (via network IPC transport)
- Distributed servers (across multiple hosts)
- Network boot (servers loaded over network)
- Server migration (move servers between hosts)
- Multiple personalities (BSD, Linux, etc.)
