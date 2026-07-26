#ifndef MANGO_COMPAT_IPC_IPC_RIGHT_H
#define MANGO_COMPAT_IPC_IPC_RIGHT_H

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/message.h>
#include <ipc/ipc_port.h>

#define ipc_right_lookup_read   ipc_right_lookup_write

extern kern_return_t ipc_right_lookup_write(ipc_space_t, mach_port_t, ipc_entry_t *);
extern boolean_t ipc_right_reverse(ipc_space_t, ipc_object_t, mach_port_t *, ipc_entry_t *);
extern kern_return_t ipc_right_dnrequest(ipc_space_t, mach_port_t, boolean_t, ipc_port_t, ipc_port_t *);
extern ipc_port_t ipc_right_dncancel(ipc_space_t, ipc_port_t, mach_port_t, ipc_entry_t);
extern boolean_t ipc_right_inuse(ipc_space_t, mach_port_t, ipc_entry_t);
extern boolean_t ipc_right_check(ipc_space_t, mach_port_t, ipc_entry_t, ipc_port_t);
extern void ipc_right_clean(ipc_space_t, mach_port_t, ipc_entry_t);
extern kern_return_t ipc_right_destroy(ipc_space_t, mach_port_t, ipc_entry_t);
extern kern_return_t ipc_right_dealloc(ipc_space_t, mach_port_t, ipc_entry_t);
extern kern_return_t ipc_right_delta(ipc_space_t, mach_port_t, ipc_entry_t, mach_port_right_t, mach_port_delta_t);
extern kern_return_t ipc_right_info(ipc_space_t, mach_port_t, ipc_entry_t, mach_port_type_t *, mach_port_urefs_t *);
extern boolean_t ipc_right_copyin_check(ipc_space_t, mach_port_t, ipc_entry_t, mach_msg_type_name_t);
extern kern_return_t ipc_right_copyin(ipc_space_t, mach_port_t, ipc_entry_t, mach_msg_type_name_t, boolean_t, ipc_object_t *, ipc_port_t *);
extern void ipc_right_copyin_undo(ipc_space_t, mach_port_t, ipc_entry_t, mach_msg_type_name_t, ipc_object_t, ipc_port_t);
extern kern_return_t ipc_right_copyin_two(ipc_space_t, mach_port_t, ipc_entry_t, ipc_object_t *, ipc_port_t *);
extern kern_return_t ipc_right_copyout(ipc_space_t, mach_port_t, ipc_entry_t, mach_msg_type_name_t, boolean_t, ipc_object_t);
extern kern_return_t ipc_right_rename(ipc_space_t, mach_port_t, ipc_entry_t, mach_port_t, ipc_entry_t);

#endif
