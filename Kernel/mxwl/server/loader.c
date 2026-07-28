#include "loader.h"
#include "registry.h"
#include "ipc.h"
#include "../mach/klog.h"
#include "../mach/mach_kernel.h"
#include "../mach/mach_port.h"
#include "../loader/mach_loader.h"
#include "../libkern/libkern.h"
#include "../kal/kal.h"

#include <string.h>

kern_return_t mx_loader_init(void)
{
    klog_sub_info("srv-load", "server loader initialized\n");
    return KERN_SUCCESS;
}

kern_return_t mx_loader_load(mx_server_t *server, const char *exec_path)
{
    if (!server || !exec_path)
        return KERN_INVALID_ARGUMENT;

    libkern_strncpy(server->path, exec_path, MX_SERVER_PATH_MAX - 1);
    mx_server_set_state(server, MX_SERVER_LOADING);

    kern_return_t kr = mx_ipc_server_endpoint_create(server);
    if (kr != KERN_SUCCESS) {
        mx_server_set_state(server, MX_SERVER_FAILED);
        return kr;
    }

    kr = mxwl_task_create(&server->task, server->name);
    if (kr != KERN_SUCCESS) {
        mx_ipc_server_endpoint_destroy(server);
        mx_server_set_state(server, MX_SERVER_FAILED);
        return kr;
    }

    kr = mxwl_thread_create(server->task, &server->thread);
    if (kr != KERN_SUCCESS) {
        mxwl_task_terminate(server->task);
        server->task = NULL;
        mx_ipc_server_endpoint_destroy(server);
        mx_server_set_state(server, MX_SERVER_FAILED);
        return kr;
    }

    klog_sub_info("srv-load", "loaded %s (task %d, thread %d)\n",
                  exec_path, server->task->id, server->thread->id);

    return KERN_SUCCESS;
}

kern_return_t mx_loader_unload(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    mx_loader_stop(server);

    if (server->thread) {
        mxwl_thread_terminate(server->thread);
        server->thread = NULL;
    }

    if (server->task) {
        mxwl_task_terminate(server->task);
        server->task = NULL;
    }

    mx_ipc_server_endpoint_destroy(server);
    mx_server_set_state(server, MX_SERVER_STOPPED);

    return KERN_SUCCESS;
}

kern_return_t mx_loader_start(mx_server_t *server)
{
    if (!server || !server->task)
        return KERN_INVALID_ARGUMENT;

    mx_server_set_state(server, MX_SERVER_STARTING);

    server->task->bootstrap_port = server->endpoint;

    char port_str[32];
    snprintf(port_str, sizeof(port_str), "%d", server->endpoint);
    kal_env_set("MXWL_SERVER_PORT", port_str, 1);
    kal_env_set("MXWL_SERVER_NAME", server->name, 1);
    kal_env_set("MXWL_SERVER_ID", port_str, 1);

    if (server->path[0] != '\0') {
        mxwl_task_t *task = server->task;
        strncpy(task->binary_path, server->path, sizeof(task->binary_path) - 1);
    }

    klog_sub_info("srv-load", "starting %s (path: %s)\n",
                  server->name, server->path);

    return KERN_SUCCESS;
}

kern_return_t mx_loader_stop(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    if (server->state == MX_SERVER_STOPPED || server->state == MX_SERVER_STOPPING)
        return KERN_SUCCESS;

    mx_server_set_state(server, MX_SERVER_STOPPING);

    if (server->task && server->task->host_pid > 0) {
        kal_process_kill(server->task->host_pid, KAL_SIGTERM);
        kal_process_wait(server->task->host_pid, NULL);
    }

    mx_server_set_state(server, MX_SERVER_STOPPED);
    return KERN_SUCCESS;
}

kern_return_t mx_loader_setup_ipc(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    return mx_ipc_server_endpoint_create(server);
}

kern_return_t mx_loader_wait_ready(mx_server_t *server, int timeout_ms)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    if (server->state == MX_SERVER_READY)
        return KERN_SUCCESS;

    int waited = 0;
    int interval = 10;

    while (waited < timeout_ms) {
        if (server->state == MX_SERVER_READY)
            return KERN_SUCCESS;
        if (server->state == MX_SERVER_FAILED || server->state == MX_SERVER_PANIC)
            return KERN_FAILURE;
        kal_usleep(interval * 1000);
        waited += interval;
    }

    return KERN_ABORTED;
}
