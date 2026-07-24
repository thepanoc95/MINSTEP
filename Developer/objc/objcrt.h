/*
 * objcrt.h - MinSTEP Objective-C Runtime
 *
 * A NeXT-inspired Objective-C runtime with Objective-C 2.0 features.
 * Designed for use with the MinSTEP objc preprocessor.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _OBJCRT_H_
#define _OBJCRT_H_

#include <stddef.h>
#include <stdarg.h>

/* ========================================================================
 * Basic Types
 * ======================================================================== */

#ifndef BOOL
typedef signed char BOOL;
#endif

#ifndef YES
#define YES ((BOOL)1)
#endif

#ifndef NO
#define NO  ((BOOL)0)
#endif

#ifndef nil
#define nil ((id)0)
#endif

#ifndef Nil
#define Nil ((Class)0)
#endif

#ifndef nil_SEL
#define nil_SEL ((SEL)0)
#endif

/* ========================================================================
 * Core Opaque Types
 * ======================================================================== */

struct objc_object;
struct objc_class;
struct objc_selector;
struct objc_method;
struct objc_ivar;
struct objc_property;
struct objc_protocol;
struct objc_category;
struct objc_method_list;
struct objc_ivar_list;
struct objc_property_list;
struct objc_protocol_list;
struct objc_super;

typedef struct objc_object   *id;
typedef struct objc_class    *Class;
typedef struct objc_selector *SEL;
typedef struct objc_method   *Method;
typedef struct objc_ivar     *Ivar;
typedef struct objc_property *Property;
typedef struct objc_protocol *Protocol;
typedef struct objc_category *Category;

typedef id (*IMP)(id, SEL, ...);
typedef void (*objc_uncaught_exception_handler)(id exception);

/* ========================================================================
 * Method Encoding Strings
 * ======================================================================== */

/* ========================================================================
 * Method Descriptor
 * ======================================================================== */

struct objc_method {
    SEL              name;
    const char      *types;
    IMP              imp;
};

struct objc_method_list {
    struct objc_method_list *_next;
    int                       _count;
    int                       _space;
    struct objc_method        _methods[1]; /* variable length */
};

/* ========================================================================
 * Ivar Descriptor
 * ======================================================================== */

struct objc_ivar {
    const char      *_name;
    const char      *_type;
    unsigned int     _offset;
};

struct objc_ivar_list {
    int                  _count;
    int                  _space;
    struct objc_ivar     _ivars[1]; /* variable length */
};

/* ========================================================================
 * Property Descriptor (Objective-C 2.0)
 * ======================================================================== */

struct objc_property {
    const char *_name;
    const char *_attributes;
};

struct objc_property_list {
    int                      _count;
    int                      _space;
    struct objc_property     _properties[1]; /* variable length */
};

/* ========================================================================
 * Protocol
 * ======================================================================== */

struct objc_protocol {
    const char                  *_name;
    struct objc_protocol_list   *_protocols;
    struct objc_method_list     *_instanceMethods;
    struct objc_method_list     *_classMethods;
    struct objc_property_list   *_properties;
};

struct objc_protocol_list {
    struct objc_protocol_list *_next;
    int                        _count;
    struct objc_protocol      *_list[1]; /* variable length */
};

/* ========================================================================
 * Class Structure
 * ======================================================================== */

struct objc_class {
    struct objc_class   *isa;
    struct objc_class   *superclass;
    const char          *name;
    long                 version;
    unsigned long        info;
    long                 instance_size;
    struct objc_ivar_list   *ivars;
    struct objc_method_list *methodLists;
    struct objc_protocol_list *protocols;
    struct objc_property_list *properties;
    struct objc_cache {
        unsigned int  _mask;
        unsigned int  _occupied;
        id           *_buckets;
    } cache;
};

/* Class info bits */
#define OBJC_CLASS_INFO_META       (1 << 0)
#define OBJC_CLASS_INFO_ROOT       (1 << 1)
#define OBJC_CLASS_INFO_NIF        (1 << 2)
#define OBJC_CLASS_INFO_SETUP      (1 << 5)
#define OBJC_CLASS_INFO_INITIALIZED (1 << 6)
#define OBJC_CLASS_INFO_FUTURE     (1 << 7)

/* ========================================================================
 * Object Structure
 * ======================================================================== */

struct objc_object {
    Class isa;
};

/* ========================================================================
 * Super Structure
 * ======================================================================== */

struct objc_super {
    id          receiver;
    Class       super_class;
};

/* ========================================================================
 * Ivar Offset Support
 * ======================================================================== */

struct objcivar_offset {
    Class       _class;
    const char *_ivar_name;
    ptrdiff_t   _offset;
};

/* ========================================================================
 * Category
 * ======================================================================== */

struct objc_category {
    const char              *_name;
    const char              *_className;
    struct objc_method_list *_instanceMethods;
    struct objc_method_list *_classMethods;
    struct objc_protocol_list *_protocols;
    struct objc_property_list *_properties;
};

/* ========================================================================
 * Message Send Functions
 * ======================================================================== */

id objc_msgSend(id self, SEL op, ...);
id objc_msgSendSuper(struct objc_super *super, SEL op, ...);
id objc_msgSend_stret(id self, SEL op, ...);
id objc_msgSendSuper_stret(struct objc_super *super, SEL op, ...);

/* ========================================================================
 * Selector Functions
 * ======================================================================== */

SEL sel_registerName(const char *name);
const char *sel_getName(SEL sel);
BOOL sel_isEqual(SEL lhs, SEL rhs);
SEL sel_getUid(const char *name);
const char *sel_getTypeEncoding(SEL sel);
void sel_setTypeEncoding(SEL sel, const char *types);

/* ========================================================================
 * Class Functions
 * ======================================================================== */

Class objc_getClass(const char *name);
Class objc_lookUpClass(const char *name);
Class objc_getMetaClass(const char *name);
Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);
void objc_registerClassPair(Class cls);
const char *class_getName(Class cls);
Class class_getSuperclass(Class cls);
BOOL class_isMetaClass(Class cls);
long class_getVersion(Class cls);
void class_setVersion(Class cls, long version);
size_t class_getInstanceSize(Class cls);

/* ========================================================================
 * Method Functions
 * ======================================================================== */

Method class_getInstanceMethod(Class cls, SEL name);
Method class_getClassMethod(Class cls, SEL name);
Method *class_copyMethodList(Class cls, unsigned int *outCount);
BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);
void class_replaceMethod(Class cls, SEL name, IMP imp, const char *types);
BOOL class_hasMethod(Class cls, SEL name);
SEL method_getName(Method m);
IMP method_getImplementation(Method m);
IMP method_setImplementation(Method m, IMP imp);
const char *method_getTypeEncoding(Method m);
unsigned int method_getNumberOfArguments(Method m);
char *method_getReturnType(Method m, char *buf, size_t bufLen);
void method_getArgumentType(Method m, unsigned int index, char *buf, size_t bufLen);
id method_invoke(id receiver, Method m, ...);

/* ========================================================================
 * Ivar Functions
 * ======================================================================== */

Ivar *class_copyIvarList(Class cls, unsigned int *outCount);
const char *ivar_getName(Ivar ivar);
const char *ivar_getTypeEncoding(Ivar ivar);
ptrdiff_t ivar_getOffset(Ivar ivar);

/* ========================================================================
 * Property Functions (Objective-C 2.0)
 * ======================================================================== */

Property *class_copyPropertyList(Class cls, unsigned int *outCount);
Property class_getProperty(Class cls, const char *name);
BOOL class_addProperty(Class cls, const char *name, const char *attributes);
void class_replaceProperty(Class cls, const char *name, const char *attributes);
const char *property_getName(Property prop);
const char *property_getAttributes(Property prop);

/* ========================================================================
 * Protocol Functions
 *
 * Note: Protocol is already a pointer type (struct objc_protocol *).
 * Functions that take/return Protocol take the pointer directly.
 * ======================================================================== */

Protocol objc_getProtocol(const char *name);
Protocol objc_allocateProtocol(const char *name);
void objc_registerProtocol(Protocol proto);
void protocol_addMethodDescription(Protocol proto, SEL name,
                                   const char *types, BOOL isRequired,
                                   BOOL isInstanceMethod);
void protocol_addProtocol(Protocol proto, Protocol addition);
void protocol_addProperty(Protocol proto, const char *name,
                          const char *attributes, BOOL isRequired,
                          BOOL isInstanceProperty);
const char *protocol_getName(Protocol proto);
BOOL protocol_conformsToProtocol(Protocol proto, Protocol other);
BOOL protocol_isEqual(Protocol proto, Protocol other);

/* Method description struct */
struct objc_method_description {
    const char *_name;
    const char *_types;
};

struct objc_method_description *protocol_copyMethodDescriptionList(
    Protocol proto, BOOL isRequiredMethod, BOOL isInstanceMethod,
    unsigned int *outCount);
Property *protocol_copyPropertyList(Protocol proto, unsigned int *outCount);
Protocol *protocol_copyProtocolList(Protocol proto, unsigned int *outCount);
struct objc_method_description protocol_getMethodDescription(
    Protocol proto, SEL sel, BOOL isRequiredMethod, BOOL isInstanceMethod);

/* ========================================================================
 * Class Addition Functions
 * ======================================================================== */

BOOL class_addIvar(Class cls, const char *name, size_t size,
                   unsigned char alignment, const char *types);
BOOL class_addProtocol(Class cls, Protocol protocol);
BOOL class_conformsToProtocol(Class cls, Protocol protocol);
BOOL class_respondsToSelector(Class cls, SEL sel);

/* ========================================================================
 * Instance/Class Creation
 * ======================================================================== */

id class_createInstance(Class cls, size_t extraBytes);
id class_createInstanceFromZone(Class cls, size_t extraBytes, void *zone);
id object_copy(id obj, size_t extraBytes);
id object_dispose(id obj);
Class object_getClass(id obj);
Class object_setClass(id obj, Class cls);

/* ========================================================================
 * Memory Management
 * ======================================================================== */

id objc_retain(id obj);
void objc_release(id obj);
id objc_autorelease(id obj);
id objc_autoreleaseReturnValue(id obj);
id objc_retainReturnValue(id obj);
void *objc_autoreleasePoolPush(void);
void objc_autoreleasePoolPop(void *pool);
unsigned long objc_retainCount(id obj);

/* ========================================================================
 * Exception Handling
 * ======================================================================== */

void objc_exception_throw(id exception);
void objc_exception_try_enter(void *context);
void objc_exception_try_exit(void *context);
id objc_exception_extract(void *context);
BOOL objc_exception_match(Class exceptionType, id exception);
objc_uncaught_exception_handler objc_setExceptionHandler(
    objc_uncaught_exception_handler handler);

/* ========================================================================
 * Fast Enumeration Support (Objective-C 2.0)
 * ======================================================================== */

typedef struct objc_enumeration_state {
    unsigned long   state;
    id             *itemsPtr;
    unsigned long  *mutationsPtr;
    unsigned long   extra[5];
} objc_enumeration_state;

void objc_enumerationMutation(id obj);

/* ========================================================================
 * Runtime Initialization
 * ======================================================================== */

void _objc_init(void);
extern BOOL _objc_runtime_ready;
int _objc_image_count(void);
int _objc_getClassList(Class *buffer, int bufferLen);

/* ========================================================================
 * Non-fragile Ivar Offset
 * ======================================================================== */

ptrdiff_t objc_offsetOf(Class cls, const char *name);

/* ========================================================================
 * Struct Return Support
 * ======================================================================== */

#if defined(__APPLE__) && defined(__arm64__)
#define OBJC_STRET 0
#elif defined(__x86_64__)
#define OBJC_STRET 0
#else
#define OBJC_STRET 1
#endif

/* ========================================================================
 * ARC Support
 * ======================================================================== */

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if __has_feature(objc_arc)
  #define OBJC_ARC 1
#else
  #define OBJC_ARC 0
#endif

#endif /* _OBJCRT_H_ */
