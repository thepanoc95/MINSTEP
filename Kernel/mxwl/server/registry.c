#include "registry.h"
#include "../mach/klog.h"
#include "../libkern/libkern.h"

#include <string.h>
#include <stdlib.h>

static mx_server_t *_registry = NULL;
static int          _registry_count = 0;

kern_return_t mx_registry_init(void)
{
    _registry = NULL;
    _registry_count = 0;
    klog_sub_info("srv", "registry initialized\n");
    return KERN_SUCCESS;
}

kern_return_t mx_server_register(mx_server_t *server)
{
    if (!server)
        return KERN_INVALID_ARGUMENT;

    if (_registry_count >= MX_SERVER_MAX) {
        klog_sub_err("srv", "registry full (%d max)\n", MX_SERVER_MAX);
        return KERN_NO_SPACE;
    }

    if (mx_server_lookup_name(server->name) != NULL) {
        klog_sub_err("srv", "server '%s' already registered\n", server->name);
        return KERN_NAME_EXISTS;
    }

    server->next = _registry;
    _registry = server;
    _registry_count++;

    char caps_str[MX_SERVER_CAPS_STR_MAX];
    mx_server_caps_string(server->capabilities, caps_str, sizeof(caps_str));

    klog_sub_info("srv", "registered: %s (id %u, caps: %s)\n",
                  server->name, server->server_id, caps_str);
    return KERN_SUCCESS;
}

kern_return_t mx_server_unregister(mx_server_t *server)
{
    if (!server || !_registry)
        return KERN_INVALID_ARGUMENT;

    mx_server_t **prev = &_registry;
    while (*prev) {
        if (*prev == server) {
            *prev = server->next;
            server->next = NULL;
            _registry_count--;
            klog_sub_info("srv", "unregistered: %s (id %u)\n",
                          server->name, server->server_id);
            return KERN_SUCCESS;
        }
        prev = &(*prev)->next;
    }

    return KERN_INVALID_OBJECT;
}

mx_server_t *mx_server_lookup_id(mx_server_id_t server_id)
{
    mx_server_t *s = _registry;
    while (s) {
        if (s->server_id == server_id)
            return s;
        s = s->next;
    }
    return NULL;
}

mx_server_t *mx_server_lookup_name(const char *name)
{
    if (!name)
        return NULL;
    mx_server_t *s = _registry;
    while (s) {
        if (libkern_strcmp(s->name, name) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}

mx_server_t *mx_server_lookup_capability(mx_capability_t cap)
{
    mx_server_t *s = _registry;
    while (s) {
        if (s->state == MX_SERVER_READY && (s->capabilities & cap))
            return s;
        s = s->next;
    }
    return NULL;
}

mx_server_t *mx_server_lookup_capability_any(uint32_t caps)
{
    mx_server_t *s = _registry;
    while (s) {
        if (s->state == MX_SERVER_READY && (s->capabilities & caps))
            return s;
        s = s->next;
    }
    return NULL;
}

kern_return_t mx_server_find(const char *query, mx_server_t **out)
{
    if (!query || !out)
        return KERN_INVALID_ARGUMENT;

    mx_server_t *s;

    s = mx_server_lookup_name(query);
    if (s) {
        *out = s;
        return KERN_SUCCESS;
    }

    long id;
    char *end;
    id = strtol(query, &end, 10);
    if (*end == '\0' && id > 0) {
        s = mx_server_lookup_id((mx_server_id_t)id);
        if (s) {
            *out = s;
            return KERN_SUCCESS;
        }
    }

    return KERN_INVALID_NAME;
}

int mx_server_list(mx_server_info_t *buf, int max)
{
    if (!buf || max <= 0)
        return 0;

    int count = 0;
    mx_server_t *s = _registry;
    while (s && count < max) {
        buf[count].server_id = s->server_id;
        libkern_strncpy(buf[count].name, s->name, MX_SERVER_NAME_MAX);
        libkern_strncpy(buf[count].version, s->version, MX_SERVER_VERSION_MAX);
        buf[count].state = s->state;
        buf[count].capabilities = s->capabilities;
        buf[count].flags = s->flags;
        buf[count].endpoint = s->endpoint;
        buf[count].pid = s->task ? s->task->host_pid : -1;
        count++;
        s = s->next;
    }

    return count;
}

int mx_server_count(void)
{
    return _registry_count;
}

mx_server_t *mx_server_first(void)
{
    return _registry;
}

mx_server_t *mx_server_next(mx_server_t *current)
{
    return current ? current->next : NULL;
}
