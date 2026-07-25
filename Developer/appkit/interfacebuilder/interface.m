/*
 * appkit/interfacebuilder/interface.m - MinSTEP Interface Builder Support
 *
 * Additional Interface Builder utilities and helper classes.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#objc
#import <appkit/interfacebuilder/nib.h>

/* ========================================================================
 * MXIBConnection - Represents an outlet or action connection
 * ======================================================================== */

@interface MXIBConnection : Object
{
    const char *label;
    const char *destination;
    const char *keyPath;
    int connectionType;  /* 0 = outlet, 1 = action */
}

+ (MXIBConnection *)outletWithLabel:(const char *)label 
                        destination:(const char *)dest 
                            keyPath:(const char *)path;
+ (MXIBConnection *)actionWithLabel:(const char *)label 
                       destination:(const char *)dest 
                           keyPath:(const char *)path;

- (const char *)label;
- (const char *)destination;
- (const char *)keyPath;
- (int)connectionType;
- (BOOL)isOutlet;
- (BOOL)isAction;

@end

/* ========================================================================
 * MXIBObjectRecord - Represents an object in the NIB
 * ======================================================================== */

@interface MXIBObjectRecord : Object
{
    const char *identifier;
    const char *className;
    const char *instanceName;
    MXIBConnection **connections;
    int connectionCount;
    NXRect frame;
}

+ (id)recordWithIdentifier:(const char *)identifier className:(const char *)className;
- (void)setClassName:(const char *)className;
- (const char *)className;
- (void)setFrame:(NXRect)rect;
- (NXRect)frame;
- (void)addConnection:(MXIBConnection *)connection;
- (MXIBConnection **)connections;
- (int)connectionCount;

@end

/* ========================================================================
 * MXIBObjectRecord Implementation
 * ======================================================================== */

@implementation MXIBConnection

+ (MXIBConnection *)outletWithLabel:(const char *)label 
                        destination:(const char *)dest 
                            keyPath:(const char *)path
{
    MXIBConnection *conn = [self new];
    conn->label = label ? strdup(label) : NULL;
    conn->destination = dest ? strdup(dest) : NULL;
    conn->keyPath = path ? strdup(path) : NULL;
    conn->connectionType = 0;
    return conn;
}

+ (MXIBConnection *)actionWithLabel:(const char *)label 
                       destination:(const char *)dest 
                           keyPath:(const char *)path
{
    MXIBConnection *conn = [self new];
    conn->label = label ? strdup(label) : NULL;
    conn->destination = dest ? strdup(dest) : NULL;
    conn->keyPath = path ? strdup(path) : NULL;
    conn->connectionType = 1;
    return conn;
}

- (const char *)label
{
    return label;
}

- (const char *)destination
{
    return destination;
}

- (const char *)keyPath
{
    return keyPath;
}

- (int)connectionType
{
    return connectionType;
}

- (BOOL)isOutlet
{
    return connectionType == 0;
}

- (BOOL)isAction
{
    return connectionType == 1;
}

@end

/* ========================================================================
 * MXIBObjectRecord Implementation
 * ======================================================================== */

@implementation MXIBObjectRecord

+ (id)recordWithIdentifier:(const char *)identifier className:(const char *)class
{
    MXIBObjectRecord *record = [self new];
    record->identifier = identifier ? strdup(identifier) : NULL;
    record->className = class ? strdup(class) : NULL;
    record->instanceName = NULL;
    record->connections = NULL;
    record->connectionCount = 0;
    record->frame = NXMakeRect(0, 0, 0, 0);
    return record;
}

- (void)setClassName:(const char *)className
{
    if (self->className) free((void *)self->className);
    self->className = className ? strdup(className) : NULL;
}

- (const char *)className
{
    return className;
}

- (void)setFrame:(NXRect)rect
{
    frame = rect;
}

- (NXRect)frame
{
    return frame;
}

- (void)addConnection:(MXIBConnection *)connection
{
    connections = (MXIBConnection **)realloc(connections, 
        (connectionCount + 1) * sizeof(MXIBConnection *));
    connections[connectionCount++] = connection;
}

- (MXIBConnection **)connections
{
    return connections;
}

- (int)connectionCount
{
    return connectionCount;
}

@end