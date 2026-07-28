#include "../mach/mach_types.h"
#include "../ipc/ipc.h"
#include "../mach/klog.h"
#include "../kal/kal.h"
#include "../libkern/libkern.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define EXAMPLE_SERVER_NAME   "example.srv"
#define EXAMPLE_SERVER_VERSION "1.0"

static int _running = 1;

static void _handle_ping(mach_port_t reply_port, uint32_t msg_id)
{
    mach_msg_t reply;
    libkern_memset(&reply, 0, sizeof(reply));
    reply.header.msgh_size = sizeof(reply);
    reply.header.msgh_id = msg_id + 1;
    reply.header.msgh_local_port = MACH_PORT_NULL;

    char *payload = "pong";
    size_t plen = libkern_strlen(payload) + 1;
    if (plen > sizeof(reply.data))
        plen = sizeof(reply.data);
    libkern_memcpy(reply.data, payload, plen);

    mach_msg_send(reply_port, &reply, reply.header.msgh_size, 1000);
}

static void _handle_get_info(mach_port_t reply_port, uint32_t msg_id)
{
    mach_msg_t reply;
    libkern_memset(&reply, 0, sizeof(reply));
    reply.header.msgh_size = sizeof(reply);
    reply.header.msgh_id = msg_id + 1;

    char info[128];
    snprintf(info, sizeof(info),
             "server=%s,version=%s,pid=%d",
             EXAMPLE_SERVER_NAME, EXAMPLE_SERVER_VERSION, getpid());

    size_t plen = libkern_strlen(info) + 1;
    if (plen > sizeof(reply.data))
        plen = sizeof(reply.data);
    libkern_memcpy(reply.data, info, plen);

    mach_msg_send(reply_port, &reply, reply.header.msgh_size, 1000);
}

int main(int argc, char *argv[])
{
    const char *server_port_str = getenv("MXWL_SERVER_PORT");
    const char *server_name = getenv("MXWL_SERVER_NAME");

    if (!server_port_str) {
        fprintf(stderr, "%s: no MXWL_SERVER_PORT set\n", EXAMPLE_SERVER_NAME);
        return 1;
    }

    mach_port_t server_port = (mach_port_t)atoi(server_port_str);

    klog_info("%s v%s starting (port %d, name: %s)\n",
              EXAMPLE_SERVER_NAME, EXAMPLE_SERVER_VERSION,
              server_port, server_name ? server_name : "(none)");

    mach_msg_t register_msg;
    libkern_memset(&register_msg, 0, sizeof(register_msg));
    register_msg.header.msgh_size = sizeof(register_msg);
    register_msg.header.msgh_id = 0x80000001;
    register_msg.header.msgh_local_port = server_port;

    char reg_data[256];
    snprintf(reg_data, sizeof(reg_data),
             "register:%s:%s:%d",
             EXAMPLE_SERVER_NAME, EXAMPLE_SERVER_VERSION, getpid());
    libkern_memcpy(register_msg.data, reg_data, libkern_strlen(reg_data) + 1);

    mach_msg_send(server_port, &register_msg, register_msg.header.msgh_size, 1000);

    klog_info("%s registered with kernel\n", EXAMPLE_SERVER_NAME);

    mach_port_t service_port = mach_port_allocate(MACH_PORT_RIGHT_RECEIVE);
    if (service_port == MACH_PORT_NULL) {
        fprintf(stderr, "%s: failed to allocate service port\n", EXAMPLE_SERVER_NAME);
        return 1;
    }

    char srv_data[64];
    snprintf(srv_data, sizeof(srv_data),
             "advertise:%d:monitor", service_port);

    mach_msg_t adv_msg;
    libkern_memset(&adv_msg, 0, sizeof(adv_msg));
    adv_msg.header.msgh_size = sizeof(adv_msg);
    adv_msg.header.msgh_id = 0x80000002;
    adv_msg.header.msgh_local_port = server_port;
    libkern_memcpy(adv_msg.data, srv_data, libkern_strlen(srv_data) + 1);

    mach_msg_send(server_port, &adv_msg, adv_msg.header.msgh_size, 1000);

    klog_info("%s service port %d advertised\n", EXAMPLE_SERVER_NAME, service_port);

    mach_msg_t ready_msg;
    libkern_memset(&ready_msg, 0, sizeof(ready_msg));
    ready_msg.header.msgh_size = sizeof(ready_msg);
    ready_msg.header.msgh_id = 0x80000003;
    ready_msg.header.msgh_local_port = server_port;
    char *ready_data = "ready";
    libkern_memcpy(ready_msg.data, ready_data, libkern_strlen(ready_data) + 1);

    mach_msg_send(server_port, &ready_msg, ready_msg.header.msgh_size, 1000);

    klog_info("%s READY, entering dispatch loop\n", EXAMPLE_SERVER_NAME);

    while (_running) {
        mach_msg_t request;
        libkern_memset(&request, 0, sizeof(request));

        kern_return_t kr = mach_msg_receive(
            service_port, &request, sizeof(request), 1000);

        if (kr == KERN_SUCCESS) {
            mach_port_t reply_port = request.header.msgh_remote_port;

            klog_info("%s received msg id=%d from port %d\n",
                      EXAMPLE_SERVER_NAME,
                      request.header.msgh_id, reply_port);

            switch (request.header.msgh_id) {
                case 1:
                    _handle_ping(reply_port, request.header.msgh_id);
                    klog_info("%s: ping -> pong\n", EXAMPLE_SERVER_NAME);
                    break;

                case 2:
                    _handle_get_info(reply_port, request.header.msgh_id);
                    klog_info("%s: info request handled\n", EXAMPLE_SERVER_NAME);
                    break;

                case 0xFFFFFFFF:
                    klog_info("%s: shutdown requested\n", EXAMPLE_SERVER_NAME);
                    _running = 0;
                    break;

                default:
                    klog_info("%s: unknown msg id=%d\n",
                              EXAMPLE_SERVER_NAME, request.header.msgh_id);
                    break;
            }
        }

        kal_usleep(100);
    }

    klog_info("%s shutting down\n", EXAMPLE_SERVER_NAME);
    mach_port_destroy(service_port);
    return 0;
}
