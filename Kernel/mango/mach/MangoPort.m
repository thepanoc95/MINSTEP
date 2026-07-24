#objc
/*
 * mango/mach/MangoPort.m
 *
 * Objective-C implementation of Mach port management.
 */

#import "MangoPort.h"
#import "klog.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

/* -----------------------------------------------------------------------
 *  Global port table (C globals, shared with C code)
 * ----------------------------------------------------------------------- */

mach_port_object_t _mango_port_table[MACH_PORT_TABLE_SIZE];
int                _mango_port_table_count = 0;

@implementation MangoPort

- (id)init {
    self = [super init];
    if (self) {
        memset(_mango_port_table, 0, sizeof(_mango_port_table));
        _mango_port_table_count = 0;
    }
    return self;
}

- (id)free {
    for (int i = 0; i < MACH_PORT_TABLE_SIZE; i++) {
        if (_mango_port_table[i].in_use) {
            [self deallocate:_mango_port_table[i].name];
        }
    }
    return [super free];
}

- (mach_port_t)allocate:(int)right {
    mach_port_object_t *obj = NULL;
    int i;

    for (i = 0; i < MACH_PORT_TABLE_SIZE; i++) {
        if (!_mango_port_table[i].in_use) {
            obj = &_mango_port_table[i];
            break;
        }
    }

    if (!obj) {
        return MACH_PORT_NULL;
    }

    memset(obj, 0, sizeof(mach_port_object_t));
    obj->name       = i + 1;
    obj->type       = MACH_PORT_TYPE_DYNAMIC;
    obj->rights     = right;
    obj->fd         = -1;
    obj->ref_count  = 1;
    obj->in_use     = YES;
    obj->queue_head = 0;
    obj->queue_tail = 0;
    obj->queue_count = 0;

    if (right == MACH_PORT_RIGHT_RECEIVE) {
        if ([self createSocketPair:obj] < 0) {
            obj->in_use = NO;
            return MACH_PORT_NULL;
        }
    }

    _mango_port_table_count++;
    return obj->name;
}

- (kern_return_t)deallocate:(mach_port_t)port {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj) return KERN_INVALID_RIGHT;

    if (obj->fd >= 0) {
        close(obj->fd);
        obj->fd = -1;
    }

    while (obj->queue_count > 0) {
        mach_msg_t *msg = [self dequeueMessage:port];
        if (msg) free(msg);
    }

    obj->in_use = NO;
    obj->ref_count--;
    _mango_port_table_count--;

    return KERN_SUCCESS;
}

- (kern_return_t)destroy:(mach_port_t)port {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj) return KERN_INVALID_RIGHT;

    obj->ref_count = 0;
    return [self deallocate:port];
}

- (mach_port_object_t *)lookup:(mach_port_t)port {
    if (port <= 0 || port > MACH_PORT_TABLE_SIZE) {
        return NULL;
    }

    mach_port_object_t *obj = &_mango_port_table[port - 1];
    if (!obj->in_use) {
        return NULL;
    }

    return obj;
}

- (int)createSocketPair:(mach_port_object_t *)obj {
    int fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        return -1;
    }

    obj->fd = fds[0];
    close(fds[1]);
    return fds[0];
}

- (kern_return_t)queueMessage:(mach_port_t)port :(mach_msg_t *)msg {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj) return KERN_INVALID_RIGHT;

    if (obj->queue_count >= MACH_PORT_QUEUE_MAX) {
        return KERN_NO_SPACE;
    }

    mach_msg_t *copy = malloc(msg->header.msgh_size);
    if (!copy) return KERN_FAILURE;

    memcpy(copy, msg, msg->header.msgh_size);

    obj->queue[obj->queue_tail] = copy;
    obj->queue_tail = (obj->queue_tail + 1) % MACH_PORT_QUEUE_MAX;
    obj->queue_count++;

    return KERN_SUCCESS;
}

- (mach_msg_t *)dequeueMessage:(mach_port_t)port {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj || obj->queue_count == 0) {
        return NULL;
    }

    mach_msg_t *msg = obj->queue[obj->queue_head];
    obj->queue[obj->queue_head] = NULL;
    obj->queue_head = (obj->queue_head + 1) % MACH_PORT_QUEUE_MAX;
    obj->queue_count--;

    return msg;
}

- (kern_return_t)insertSend:(mach_port_t)port :(mach_port_name_t)name {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj) return KERN_INVALID_RIGHT;
    obj->rights = MACH_PORT_RIGHT_SEND;
    return KERN_SUCCESS;
}

- (kern_return_t)insertReceive:(mach_port_t)port :(mach_port_name_t)name {
    mach_port_object_t *obj = [self lookup:port];
    if (!obj) return KERN_INVALID_RIGHT;
    obj->rights = MACH_PORT_RIGHT_RECEIVE;
    return KERN_SUCCESS;
}

@end
