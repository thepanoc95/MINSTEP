/*
 * objcrt.c - MinSTEP Objective-C Runtime Implementation
 *
 * A NeXT-inspired Objective-C runtime with Objective-C 2.0 features.
 * All-C implementation with no assembly.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#include "objcrt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/* ========================================================================
 * Forward Declarations
 * ======================================================================== */

static void _objc_resolve_class(Class cls);
static IMP _objc_lookup_method(Class cls, SEL sel);
static void _objc_init_class(Class cls);

/* ========================================================================
 * Global State
 * ======================================================================== */

BOOL _objc_runtime_ready = 0;

#define CLASS_HASH_SIZE 256
static Class _class_hash[CLASS_HASH_SIZE];

#define SEL_HASH_SIZE 512
static SEL _sel_hash[SEL_HASH_SIZE];

static unsigned int _sel_count = 0;
static unsigned int _class_count = 0;

#define AUTORELEASE_POOL_SIZE 4096
static id _autorelease_pool[AUTORELEASE_POOL_SIZE];
static int _autorelease_pool_top = 0;

static objc_uncaught_exception_handler _exception_handler = NULL;

/* ========================================================================
 * Hash Functions
 * ======================================================================== */

static unsigned int _hash_string(const char *str)
{
    unsigned int hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)*str;
        str++;
    }
    return hash;
}

/* ========================================================================
 * Selector Management
 * ======================================================================== */

struct objc_selector {
    const char  *_name;
    const char  *_types;
    unsigned int _uid;
};

SEL sel_registerName(const char *name)
{
    unsigned int idx;
    SEL sel;

    if (!name) return nil_SEL;

    idx = _hash_string(name) & (SEL_HASH_SIZE - 1);
    while (_sel_hash[idx]) {
        if (_sel_hash[idx]->_name && strcmp(_sel_hash[idx]->_name, name) == 0)
            return _sel_hash[idx];
        idx = (idx + 1) & (SEL_HASH_SIZE - 1);
    }

    sel = (SEL)calloc(1, sizeof(struct objc_selector));
    sel->_name = strdup(name);
    sel->_types = NULL;
    sel->_uid = _sel_count++;

    idx = _hash_string(name) & (SEL_HASH_SIZE - 1);
    while (_sel_hash[idx]) {
        idx = (idx + 1) & (SEL_HASH_SIZE - 1);
    }
    _sel_hash[idx] = sel;

    return sel;
}

SEL sel_getUid(const char *name) { return sel_registerName(name); }

const char *sel_getName(SEL sel)
{
    if (!sel) return NULL;
    return sel->_name;
}

BOOL sel_isEqual(SEL lhs, SEL rhs)
{
    if (lhs == rhs) return YES;
    if (!lhs || !rhs) return NO;
    return strcmp(lhs->_name, rhs->_name) == 0;
}

const char *sel_getTypeEncoding(SEL sel) { return sel ? sel->_types : NULL; }
void sel_setTypeEncoding(SEL sel, const char *types) { if (sel) sel->_types = types; }

/* ========================================================================
 * Class Management
 * ======================================================================== */

static Class _alloc_class(void)
{
    Class cls = (Class)calloc(1, sizeof(struct objc_class));
    cls->version = 0;
    cls->methodLists = NULL;
    cls->ivars = NULL;
    cls->protocols = NULL;
    cls->properties = NULL;
    return cls;
}

Class objc_getClass(const char *name)
{
    Class cls;
    if (!name) return Nil;
    cls = objc_lookUpClass(name);
    if (!cls) fprintf(stderr, "objc: class '%s' not found\n", name);
    return cls;
}

Class objc_lookUpClass(const char *name)
{
    unsigned int idx;
    if (!name || !_objc_runtime_ready) return Nil;
    idx = _hash_string(name) & (CLASS_HASH_SIZE - 1);
    while (_class_hash[idx]) {
        if (_class_hash[idx]->name && strcmp(_class_hash[idx]->name, name) == 0)
            return _class_hash[idx];
        idx = (idx + 1) & (CLASS_HASH_SIZE - 1);
    }
    return NULL;
}

Class objc_getMetaClass(const char *name)
{
    Class cls = objc_getClass(name);
    return cls ? cls->isa : Nil;
}

const char *class_getName(Class cls) { return cls ? cls->name : "<nil>"; }
Class class_getSuperclass(Class cls) { return cls ? cls->superclass : Nil; }
BOOL class_isMetaClass(Class cls) { return cls ? (cls->info & OBJC_CLASS_INFO_META) : NO; }
long class_getVersion(Class cls) { return cls ? cls->version : 0; }
void class_setVersion(Class cls, long version) { if (cls) cls->version = version; }

size_t class_getInstanceSize(Class cls)
{
    size_t size;
    if (!cls) return 0;
    size = cls->instance_size;
    if (size < (long)sizeof(id))
        size = sizeof(id);
    return size;
}

/* ========================================================================
 * Class Pair Creation
 * ======================================================================== */

Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes)
{
    Class cls, meta;

    if (!name) return Nil;

    cls = _alloc_class();
    cls->name = strdup(name);
    cls->superclass = superclass;
    cls->instance_size = sizeof(struct objc_object);
    cls->info = OBJC_CLASS_INFO_NIF;

    if (superclass)
        cls->instance_size = superclass->instance_size + (long)extraBytes;

    meta = _alloc_class();
    meta->name = strdup(name);
    meta->superclass = superclass ? superclass->isa : NULL;
    meta->info = OBJC_CLASS_INFO_META;
    meta->instance_size = sizeof(struct objc_class);

    cls->isa = meta;
    meta->isa = meta;

    return cls;
}

void objc_registerClassPair(Class cls)
{
    unsigned int idx;
    if (!cls) return;

    idx = _hash_string(cls->name) & (CLASS_HASH_SIZE - 1);
    while (_class_hash[idx]) idx = (idx + 1) & (CLASS_HASH_SIZE - 1);
    _class_hash[idx] = cls;
    _class_count++;

    if (cls->isa) {
        unsigned int midx = _hash_string(cls->name) & (CLASS_HASH_SIZE - 1);
        while (_class_hash[midx]) midx = (midx + 1) & (CLASS_HASH_SIZE - 1);
        _class_hash[midx] = cls->isa;
        cls->isa->info |= OBJC_CLASS_INFO_SETUP;
    }

    cls->info |= OBJC_CLASS_INFO_SETUP;
    _objc_init_class(cls);
}

/* ========================================================================
 * Method Management
 * ======================================================================== */

static Method _method_list_find(struct objc_method_list *list, SEL name)
{
    int i;
    if (!list) return NULL;
    for (i = 0; i < list->_count; i++) {
        if (list->_methods[i].name == name ||
            (list->_methods[i].name && name &&
             strcmp(list->_methods[i].name->_name, name->_name) == 0)) {
            return &list->_methods[i];
        }
    }
    return NULL;
}

Method class_getInstanceMethod(Class cls, SEL name)
{
    Class cur;
    if (!cls || !name) return NULL;
    for (cur = cls; cur; cur = cur->superclass) {
        struct objc_method_list *list = cur->methodLists;
        while (list) {
            Method m = _method_list_find(list, name);
            if (m) return m;
            list = list->_next;
        }
    }
    return NULL;
}

Method class_getClassMethod(Class cls, SEL name)
{
    return cls ? class_getInstanceMethod(cls->isa, name) : NULL;
}

Method *class_copyMethodList(Class cls, unsigned int *outCount)
{
    unsigned int count = 0;
    Method *result;
    struct objc_method_list *list;
    int i, total = 0;

    if (!cls) { if (outCount) *outCount = 0; return NULL; }

    list = cls->methodLists;
    while (list) { total += list->_count; list = list->_next; }
    if (total == 0) { if (outCount) *outCount = 0; return NULL; }

    result = (Method *)malloc(total * sizeof(Method));
    if (!result) { if (outCount) *outCount = 0; return NULL; }

    list = cls->methodLists;
    while (list) {
        for (i = 0; i < list->_count && count < (unsigned int)total; i++)
            result[count++] = &list->_methods[i];
        list = list->_next;
    }
    if (outCount) *outCount = count;
    return result;
}

BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types)
{
    struct objc_method_list *list;
    Method existing;
    if (!cls || !name || !imp) return NO;
    existing = class_getInstanceMethod(cls, name);
    if (existing) return NO;

    list = cls->methodLists;
    if (!list || list->_space <= 0) {
        list = (struct objc_method_list *)calloc(
            1, sizeof(struct objc_method_list) + sizeof(struct objc_method) * 4);
        list->_count = 0;
        list->_space = 4;
        list->_next = cls->methodLists;
        cls->methodLists = list;
    }
    list->_methods[list->_count].name = name;
    list->_methods[list->_count].types = types;
    list->_methods[list->_count].imp = imp;
    list->_count++;
    list->_space--;
    return YES;
}

void class_replaceMethod(Class cls, SEL name, IMP imp, const char *types)
{
    Method m;
    if (!cls || !name || !imp) return;
    m = class_getInstanceMethod(cls, name);
    if (m) { m->imp = imp; if (types) m->types = types; }
    else class_addMethod(cls, name, imp, types);
}

BOOL class_hasMethod(Class cls, SEL name)
{
    return class_getInstanceMethod(cls, name) != NULL;
}

SEL method_getName(Method m) { return m ? m->name : NULL; }
IMP method_getImplementation(Method m) { return m ? m->imp : NULL; }

IMP method_setImplementation(Method m, IMP imp)
{
    IMP old;
    if (!m) return NULL;
    old = m->imp;
    m->imp = imp;
    return old;
}

const char *method_getTypeEncoding(Method m) { return m ? m->types : NULL; }

unsigned int method_getNumberOfArguments(Method m)
{
    const char *types;
    unsigned int count = 2;
    if (!m || !m->types) return count;
    types = m->types;
    while (*types && *types != '@' && *types != ':') types++;
    if (*types == '@') types++;
    if (*types == '@') types++;
    if (*types == ':') types++;
    while (*types) {
        count++;
        types++;
    }
    return count;
}

char *method_getReturnType(Method m, char *buf, size_t bufLen)
{
    const char *types;
    size_t len;
    if (!m || !m->types || !buf || bufLen == 0) {
        if (buf && bufLen > 0) buf[0] = '\0';
        return buf;
    }
    types = m->types;
    len = strlen(types);
    if (len < bufLen) { memcpy(buf, types, len); buf[len] = '\0'; }
    else { memcpy(buf, types, bufLen - 1); buf[bufLen - 1] = '\0'; }
    return buf;
}

void method_getArgumentType(Method m, unsigned int index, char *buf, size_t bufLen)
{
    const char *types;
    if (!m || !m->types || !buf || bufLen == 0) {
        if (buf && bufLen > 0) buf[0] = '\0';
        return;
    }
    types = m->types;
    while (*types && *types != '@' && *types != ':') types++;
    if (index == 0) { snprintf(buf, bufLen, "@"); return; }
    if (index == 1) { snprintf(buf, bufLen, ":"); return; }
    if (*types == '@') types++;
    if (*types == ':') types++;
    {
        unsigned int skip = index - 2;
        while (skip > 0 && *types) { types++; skip--; }
    }
    if (*types) { buf[0] = *types; buf[1] = '\0'; }
    else buf[0] = '\0';
}

id method_invoke(id receiver, Method m, ...)
{
    va_list args;
    id result;
    if (!m || !m->imp) return nil;
    va_start(args, m);
    result = ((id(*)(id, SEL, va_list))m->imp)(receiver, m->name, args);
    va_end(args);
    return result;
}

/* ========================================================================
 * Ivar Management
 * ======================================================================== */

Ivar *class_copyIvarList(Class cls, unsigned int *outCount)
{
    Ivar *result;
    unsigned int i, count;
    if (!cls || !cls->ivars) { if (outCount) *outCount = 0; return NULL; }
    count = (unsigned int)cls->ivars->_count;
    if (count == 0) { if (outCount) *outCount = 0; return NULL; }
    result = (Ivar *)malloc(count * sizeof(Ivar));
    for (i = 0; i < count; i++) result[i] = &cls->ivars->_ivars[i];
    if (outCount) *outCount = count;
    return result;
}

const char *ivar_getName(Ivar ivar) { return ivar ? ivar->_name : NULL; }
const char *ivar_getTypeEncoding(Ivar ivar) { return ivar ? ivar->_type : NULL; }
ptrdiff_t ivar_getOffset(Ivar ivar) { return ivar ? (ptrdiff_t)ivar->_offset : 0; }

BOOL class_addIvar(Class cls, const char *name, size_t size,
                   unsigned char alignment, const char *types)
{
    struct objc_ivar_list *list;
    unsigned int count;
    size_t offset;

    if (!cls || !name) return NO;
    if (!(cls->info & OBJC_CLASS_INFO_NIF)) return NO;

    if (!cls->ivars) {
        cls->ivars = (struct objc_ivar_list *)calloc(
            1, sizeof(struct objc_ivar_list) + sizeof(struct objc_ivar) * 4);
        cls->ivars->_count = 0;
        cls->ivars->_space = 4;
    }

    list = cls->ivars;
    if (list->_space <= 0) {
        unsigned int new_count = (unsigned int)list->_count + 4;
        struct objc_ivar_list *nl;
        nl = (struct objc_ivar_list *)realloc(list,
            sizeof(struct objc_ivar_list) + sizeof(struct objc_ivar) * new_count);
        if (!nl) return NO;
        nl->_space = 4;
        cls->ivars = nl;
        list = nl;
    }

    count = (unsigned int)list->_count;
    offset = cls->instance_size;
    if (alignment > 0) {
        size_t align = 1UL << alignment;
        offset = (offset + align - 1) & ~(align - 1);
    }
    list->_ivars[count]._name = strdup(name);
    list->_ivars[count]._type = types ? strdup(types) : NULL;
    list->_ivars[count]._offset = (unsigned int)offset;
    list->_count++;
    list->_space--;
    cls->instance_size = (long)(offset + size);
    return YES;
}

ptrdiff_t objc_offsetOf(Class cls, const char *name)
{
    unsigned int i;
    if (!cls || !cls->ivars || !name) return 0;
    for (i = 0; i < (unsigned)cls->ivars->_count; i++) {
        if (strcmp(cls->ivars->_ivars[i]._name, name) == 0)
            return (ptrdiff_t)cls->ivars->_ivars[i]._offset;
    }
    return 0;
}

/* ========================================================================
 * Property Management (Objective-C 2.0)
 * ======================================================================== */

Property *class_copyPropertyList(Class cls, unsigned int *outCount)
{
    Property *result;
    unsigned int i, count;
    if (!cls || !cls->properties) { if (outCount) *outCount = 0; return NULL; }
    count = (unsigned int)cls->properties->_count;
    if (count == 0) { if (outCount) *outCount = 0; return NULL; }
    result = (Property *)malloc(count * sizeof(Property));
    for (i = 0; i < count; i++) result[i] = &cls->properties->_properties[i];
    if (outCount) *outCount = count;
    return result;
}

Property class_getProperty(Class cls, const char *name)
{
    unsigned int i;
    if (!cls || !cls->properties || !name) return NULL;
    for (i = 0; i < (unsigned)cls->properties->_count; i++) {
        if (strcmp(cls->properties->_properties[i]._name, name) == 0)
            return &cls->properties->_properties[i];
    }
    return NULL;
}

BOOL class_addProperty(Class cls, const char *name, const char *attributes)
{
    struct objc_property_list *list;
    if (!cls || !name) return NO;
    if (!cls->properties) {
        cls->properties = (struct objc_property_list *)calloc(
            1, sizeof(struct objc_property_list) + sizeof(struct objc_property) * 4);
        cls->properties->_count = 0;
        cls->properties->_space = 4;
    }
    list = cls->properties;
    if (list->_space <= 0) {
        unsigned int nc = (unsigned int)list->_count + 4;
        struct objc_property_list *nl;
        nl = (struct objc_property_list *)realloc(list,
            sizeof(struct objc_property_list) + sizeof(struct objc_property) * nc);
        if (!nl) return NO;
        nl->_space = 4;
        cls->properties = nl;
        list = nl;
    }
    list->_properties[list->_count]._name = strdup(name);
    list->_properties[list->_count]._attributes = attributes ? strdup(attributes) : NULL;
    list->_count++;
    list->_space--;
    return YES;
}

void class_replaceProperty(Class cls, const char *name, const char *attributes)
{
    unsigned int i;
    if (!cls || !cls->properties || !name) return;
    for (i = 0; i < (unsigned)cls->properties->_count; i++) {
        if (strcmp(cls->properties->_properties[i]._name, name) == 0) {
            if (cls->properties->_properties[i]._attributes)
                free((void *)cls->properties->_properties[i]._attributes);
            cls->properties->_properties[i]._attributes =
                attributes ? strdup(attributes) : NULL;
            return;
        }
    }
    class_addProperty(cls, name, attributes);
}

const char *property_getName(Property prop) { return prop ? prop->_name : NULL; }
const char *property_getAttributes(Property prop) { return prop ? prop->_attributes : NULL; }

/* ========================================================================
 * Protocol Management
 * ======================================================================== */

#define PROTOCOL_HASH_SIZE 64
static Protocol _protocol_hash[PROTOCOL_HASH_SIZE];

Protocol objc_allocateProtocol(const char *name)
{
    Protocol proto;
    unsigned int idx;
    if (!name) return NULL;

    proto = (Protocol)calloc(1, sizeof(struct objc_protocol));
    proto->_name = strdup(name);

    idx = _hash_string(name) & (PROTOCOL_HASH_SIZE - 1);
    while (_protocol_hash[idx]) idx = (idx + 1) & (PROTOCOL_HASH_SIZE - 1);
    _protocol_hash[idx] = proto;

    return proto;
}

void objc_registerProtocol(Protocol proto) { (void)proto; }

Protocol objc_getProtocol(const char *name)
{
    unsigned int idx;
    if (!name) return NULL;
    idx = _hash_string(name) & (PROTOCOL_HASH_SIZE - 1);
    while (_protocol_hash[idx]) {
        if (_protocol_hash[idx]->_name && strcmp(_protocol_hash[idx]->_name, name) == 0)
            return _protocol_hash[idx];
        idx = (idx + 1) & (PROTOCOL_HASH_SIZE - 1);
    }
    return NULL;
}

void protocol_addMethodDescription(Protocol proto, SEL name,
                                   const char *types, BOOL isRequired,
                                   BOOL isInstanceMethod)
{
    struct objc_method_list **listp;
    struct objc_method_list *list;

    (void)isRequired;
    if (!proto || !name) return;

    listp = isInstanceMethod ? &proto->_instanceMethods : &proto->_classMethods;

    if (!*listp) {
        *listp = (struct objc_method_list *)calloc(
            1, sizeof(struct objc_method_list) + sizeof(struct objc_method) * 4);
        (*listp)->_count = 0;
        (*listp)->_space = 4;
    }
    list = *listp;
    if (list->_space <= 0) {
        unsigned int nc = (unsigned)list->_count + 4;
        list = (struct objc_method_list *)realloc(list,
            sizeof(struct objc_method_list) + sizeof(struct objc_method) * nc);
        list->_space = 4;
        *listp = list;
    }
    list->_methods[list->_count].name = name;
    list->_methods[list->_count].types = types;
    list->_methods[list->_count].imp = NULL;
    list->_count++;
    list->_space--;
}

void protocol_addProtocol(Protocol proto, Protocol addition)
{
    struct objc_protocol_list *plist;
    if (!proto || !addition) return;
    if (!proto->_protocols) {
        proto->_protocols = (struct objc_protocol_list *)calloc(
            1, sizeof(struct objc_protocol_list) + sizeof(Protocol) * 4);
        proto->_protocols->_count = 0;
    }
    plist = proto->_protocols;
    plist->_list[plist->_count++] = addition;
}

void protocol_addProperty(Protocol proto, const char *name,
                          const char *attributes, BOOL isRequired,
                          BOOL isInstanceProperty)
{
    struct objc_property_list *list;
    (void)isRequired; (void)isInstanceProperty;
    if (!proto || !name) return;
    if (!proto->_properties) {
        proto->_properties = (struct objc_property_list *)calloc(
            1, sizeof(struct objc_property_list) + sizeof(struct objc_property) * 4);
        proto->_properties->_count = 0;
        proto->_properties->_space = 4;
    }
    list = proto->_properties;
    if (list->_space <= 0) {
        unsigned int nc = (unsigned)list->_count + 4;
        list = (struct objc_property_list *)realloc(list,
            sizeof(struct objc_property_list) + sizeof(struct objc_property) * nc);
        list->_space = 4;
        proto->_properties = list;
    }
    list->_properties[list->_count]._name = strdup(name);
    list->_properties[list->_count]._attributes = attributes ? strdup(attributes) : NULL;
    list->_count++;
    list->_space--;
}

const char *protocol_getName(Protocol proto) { return proto ? proto->_name : NULL; }

BOOL protocol_conformsToProtocol(Protocol proto, Protocol other)
{
    unsigned int i;
    if (!proto || !other) return NO;
    if (proto == other) return YES;
    if (proto->_protocols) {
        for (i = 0; i < (unsigned)proto->_protocols->_count; i++) {
            if (protocol_conformsToProtocol(proto->_protocols->_list[i], other))
                return YES;
        }
    }
    return NO;
}

BOOL protocol_isEqual(Protocol proto, Protocol other)
{
    if (!proto || !other) return NO;
    if (proto == other) return YES;
    return strcmp(proto->_name, other->_name) == 0;
}

struct objc_method_description *protocol_copyMethodDescriptionList(
    Protocol proto, BOOL isRequiredMethod, BOOL isInstanceMethod,
    unsigned int *outCount)
{
    struct objc_method_list *list;
    struct objc_method_description *result;
    unsigned int i, count;

    (void)isRequiredMethod;
    if (!proto) { if (outCount) *outCount = 0; return NULL; }

    list = isInstanceMethod ? proto->_instanceMethods : proto->_classMethods;
    if (!list) { if (outCount) *outCount = 0; return NULL; }

    count = (unsigned)list->_count;
    if (count == 0) { if (outCount) *outCount = 0; return NULL; }

    result = (struct objc_method_description *)malloc(
        count * sizeof(struct objc_method_description));
    for (i = 0; i < count; i++) {
        result[i]._name = sel_getName(list->_methods[i].name);
        result[i]._types = list->_methods[i].types;
    }
    if (outCount) *outCount = count;
    return result;
}

Property *protocol_copyPropertyList(Protocol proto, unsigned int *outCount)
{
    Property *result;
    unsigned int i, count;
    if (!proto || !proto->_properties) { if (outCount) *outCount = 0; return NULL; }
    count = (unsigned)proto->_properties->_count;
    if (count == 0) { if (outCount) *outCount = 0; return NULL; }
    result = (Property *)malloc(count * sizeof(Property));
    for (i = 0; i < count; i++) result[i] = &proto->_properties->_properties[i];
    if (outCount) *outCount = count;
    return result;
}

Protocol *protocol_copyProtocolList(Protocol proto, unsigned int *outCount)
{
    Protocol *result;
    unsigned int i;
    if (!proto || !proto->_protocols) { if (outCount) *outCount = 0; return NULL; }
    result = (Protocol *)malloc((unsigned)proto->_protocols->_count * sizeof(Protocol));
    for (i = 0; i < (unsigned)proto->_protocols->_count; i++)
        result[i] = proto->_protocols->_list[i];
    if (outCount) *outCount = (unsigned)proto->_protocols->_count;
    return result;
}

struct objc_method_description protocol_getMethodDescription(
    Protocol proto, SEL sel, BOOL isRequiredMethod, BOOL isInstanceMethod)
{
    struct objc_method_list *list;
    unsigned int i;
    struct objc_method_description empty = { NULL, NULL };

    (void)isRequiredMethod;
    if (!proto || !sel) return empty;
    list = isInstanceMethod ? proto->_instanceMethods : proto->_classMethods;
    if (!list) return empty;
    for (i = 0; i < (unsigned)list->_count; i++) {
        if (list->_methods[i].name == sel) {
            struct objc_method_description desc;
            desc._name = sel_getName(sel);
            desc._types = list->_methods[i].types;
            return desc;
        }
    }
    return empty;
}

/* ========================================================================
 * Class Protocol & Conformance
 * ======================================================================== */

BOOL class_addProtocol(Class cls, Protocol protocol)
{
    struct objc_protocol_list *list;
    if (!cls || !protocol) return NO;
    if (!cls->protocols) {
        cls->protocols = (struct objc_protocol_list *)calloc(
            1, sizeof(struct objc_protocol_list) + sizeof(Protocol) * 4);
        cls->protocols->_count = 0;
    }
    list = cls->protocols;
    list->_list[list->_count++] = protocol;
    return YES;
}

BOOL class_conformsToProtocol(Class cls, Protocol protocol)
{
    unsigned int i;
    if (!cls || !protocol) return NO;
    if (cls->protocols) {
        for (i = 0; i < (unsigned)cls->protocols->_count; i++) {
            if (protocol_conformsToProtocol(cls->protocols->_list[i], protocol))
                return YES;
        }
    }
    if (cls->superclass)
        return class_conformsToProtocol(cls->superclass, protocol);
    return NO;
}

BOOL class_respondsToSelector(Class cls, SEL sel)
{
    return (cls && sel) ? (class_getInstanceMethod(cls, sel) != NULL) : NO;
}

/* ========================================================================
 * Instance Creation
 * ======================================================================== */

id class_createInstance(Class cls, size_t extraBytes)
{
    id obj;
    size_t size;
    if (!cls) return NULL;
    size = class_getInstanceSize(cls) + extraBytes;
    obj = (id)calloc(1, size);
    if (!obj) return NULL;
    obj->isa = cls;
    return obj;
}

id class_createInstanceFromZone(Class cls, size_t extraBytes, void *zone)
{
    (void)zone;
    return class_createInstance(cls, extraBytes);
}

id object_copy(id obj, size_t extraBytes)
{
    id new_obj;
    size_t size;
    if (!obj) return NULL;
    size = class_getInstanceSize(object_getClass(obj)) + extraBytes;
    new_obj = (id)malloc(size);
    if (!new_obj) return NULL;
    memcpy(new_obj, obj, size - extraBytes);
    return new_obj;
}

id object_dispose(id obj)
{
    if (!obj) return NULL;
    free(obj);
    return NULL;
}

Class object_getClass(id obj) { return obj ? obj->isa : Nil; }

Class object_setClass(id obj, Class cls)
{
    Class old;
    if (!obj) return Nil;
    old = obj->isa;
    obj->isa = cls;
    return old;
}

/* ========================================================================
 * Message Dispatch
 * ======================================================================== */

static IMP _objc_lookup_method(Class cls, SEL sel)
{
    Method m;
    Class cur;
    if (!cls || !sel) return NULL;
    for (cur = cls; cur; cur = cur->superclass) {
        struct objc_method_list *list = cur->methodLists;
        while (list) {
            m = _method_list_find(list, sel);
            if (m && m->imp) return m->imp;
            list = list->_next;
        }
    }
    return NULL;
}

static void _objc_resolve_class(Class cls)
{
    if (!cls || (cls->info & OBJC_CLASS_INFO_SETUP)) return;
    if (cls->superclass && !(cls->superclass->info & OBJC_CLASS_INFO_SETUP))
        _objc_resolve_class(cls->superclass);
    cls->info |= OBJC_CLASS_INFO_SETUP;
}

static void _objc_init_class(Class cls)
{
    if (cls) _objc_resolve_class(cls);
}

id objc_msgSend(id self, SEL op, ...)
{
    IMP imp;
    va_list args;
    id result;
    if (!self || !op) return nil;
    imp = _objc_lookup_method(self->isa, op);
    if (!imp) {
        fprintf(stderr, "objc: unrecognized selector '%s' sent to %s %p\n",
                sel_getName(op), class_getName(self->isa), (void *)self);
        abort();
    }
    va_start(args, op);
    result = imp(self, op, args);
    va_end(args);
    return result;
}

id objc_msgSendSuper(struct objc_super *super, SEL op, ...)
{
    IMP imp;
    va_list args;
    id result;
    if (!super || !super->receiver || !op) return nil;
    imp = _objc_lookup_method(super->super_class, op);
    if (!imp) {
        fprintf(stderr, "objc: unrecognized selector '%s' sent to super of %s\n",
                sel_getName(op), class_getName(super->receiver->isa));
        abort();
    }
    va_start(args, op);
    result = imp(super->receiver, op, args);
    va_end(args);
    return result;
}

id objc_msgSend_stret(id self, SEL op, ...) { return objc_msgSend(self, op); }
id objc_msgSendSuper_stret(struct objc_super *super, SEL op, ...) { return objc_msgSendSuper(super, op); }

/* ========================================================================
 * Memory Management
 * ======================================================================== */

#define RC_HASH_SIZE 1024
static struct {
    id             obj;
    unsigned long  retainCount;
} _rc_table[RC_HASH_SIZE];

static unsigned long *_rc_find(id obj)
{
    unsigned int idx = (unsigned int)((unsigned long)obj >> 4) & (RC_HASH_SIZE - 1);
    while (_rc_table[idx].obj) {
        if (_rc_table[idx].obj == obj) return &_rc_table[idx].retainCount;
        idx = (idx + 1) & (RC_HASH_SIZE - 1);
    }
    _rc_table[idx].obj = obj;
    _rc_table[idx].retainCount = 1;
    return &_rc_table[idx].retainCount;
}

id objc_retain(id obj)
{
    unsigned long *count;
    if (!obj) return nil;
    count = _rc_find(obj);
    (*count)++;
    return obj;
}

void objc_release(id obj)
{
    unsigned long *count;
    if (!obj) return;
    count = _rc_find(obj);
    if (*count > 0) (*count)--;
    if (*count == 0) objc_msgSend(obj, sel_registerName("dealloc"));
}

id objc_autorelease(id obj)
{
    if (!obj) return nil;
    if (_autorelease_pool_top < AUTORELEASE_POOL_SIZE)
        _autorelease_pool[_autorelease_pool_top++] = obj;
    return obj;
}

id objc_autoreleaseReturnValue(id obj) { return objc_autorelease(obj); }
id objc_retainReturnValue(id obj) { return objc_retain(obj); }

void *objc_autoreleasePoolPush(void) { return (void *)(long)_autorelease_pool_top; }

void objc_autoreleasePoolPop(void *pool)
{
    int popTo = (int)(long)pool;
    while (_autorelease_pool_top > popTo) {
        id obj = _autorelease_pool[--_autorelease_pool_top];
        objc_release(obj);
    }
}

unsigned long objc_retainCount(id obj)
{
    unsigned long *count;
    if (!obj) return 0;
    count = _rc_find(obj);
    return *count;
}

/* ========================================================================
 * Exception Handling
 * ======================================================================== */

void objc_exception_throw(id exception)
{
    if (_exception_handler) _exception_handler(exception);
    else { fprintf(stderr, "objc: uncaught exception: %p\n", (void *)exception); abort(); }
}

void objc_exception_try_enter(void *context) { (void)context; }
void objc_exception_try_exit(void *context) { (void)context; }
id objc_exception_extract(void *context) { (void)context; return nil; }
BOOL objc_exception_match(Class exceptionType, id exception)
{ (void)exceptionType; (void)exception; return NO; }

objc_uncaught_exception_handler objc_setExceptionHandler(
    objc_uncaught_exception_handler handler)
{
    objc_uncaught_exception_handler old = _exception_handler;
    _exception_handler = handler;
    return old;
}

/* ========================================================================
 * Fast Enumeration Support
 * ======================================================================== */

void objc_enumerationMutation(id obj)
{
    (void)obj;
    fprintf(stderr, "objc: collection was mutated during fast enumeration\n");
    abort();
}

/* ========================================================================
 * Runtime Initialization
 * ======================================================================== */

void _objc_init(void)
{
    if (_objc_runtime_ready) return;
    memset(_class_hash, 0, sizeof(_class_hash));
    memset(_sel_hash, 0, sizeof(_sel_hash));
    memset(_rc_table, 0, sizeof(_rc_table));
    memset(_autorelease_pool, 0, sizeof(_autorelease_pool));
    _sel_count = 0;
    _class_count = 0;
    _autorelease_pool_top = 0;
    _objc_runtime_ready = 1;
}

int _objc_image_count(void) { return 1; }

int _objc_getClassList(Class *buffer, int bufferLen)
{
    int count = 0;
    unsigned int i;
    if (!buffer || bufferLen <= 0) return _class_count;
    for (i = 0; i < CLASS_HASH_SIZE && count < bufferLen; i++) {
        if (_class_hash[i] && !(_class_hash[i]->info & OBJC_CLASS_INFO_META))
            buffer[count++] = _class_hash[i];
    }
    return count;
}

__attribute__((constructor))
static void _objc_runtime_constructor(void) { _objc_init(); }
