/*
 * objc/libobjcrt.h - MinSTEP Objective-C Runtime Header
 *
 * Complete type definitions for the MinSTEP Objective-C runtime.
 * This header defines all types, structs, constants, and function
 * declarations needed by both the runtime implementation (objcrt.c)
 * and user code compiled with the preprocessor.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _OBJC_LIBOBJCRT_H_
#define _OBJC_LIBOBJCRT_H_

#include <stddef.h>
#include <stdarg.h>

/* ========================================================================
 * Forward Declarations (for pointer typedefs)
 * ======================================================================== */

struct objc_class;
struct objc_object;
struct objc_selector;
struct objc_method;
struct objc_method_list;
struct objc_ivar;
struct objc_ivar_list;
struct objc_property;
struct objc_property_list;
struct objc_protocol;
struct objc_protocol_list;

/* ========================================================================
 * Primitive Type Definitions
 * ======================================================================== */

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef YES
#define YES ((BOOL)1)
#endif

#ifndef NO
#define NO  ((BOOL)0)
#endif

typedef struct objc_object *id;
typedef struct objc_class  *Class;
typedef struct objc_selector *SEL;
typedef id (*IMP)(id, SEL, ...);
typedef struct objc_method      *Method;
typedef struct objc_ivar        *Ivar;
typedef struct objc_protocol    *Protocol;
typedef struct objc_property    *Property;

/* ========================================================================
 * Nil Constants
 * ======================================================================== */

#ifndef nil
#define nil ((id)0)
#endif

#ifndef Nil
#define Nil ((Class)0)
#endif

#define nil_SEL ((SEL)0)

/* ========================================================================
 * Class Info Flags
 * ======================================================================== */

#define OBJC_CLASS_INFO_NIF     (1 << 0)
#define OBJC_CLASS_INFO_SETUP   (1 << 1)
#define OBJC_CLASS_INFO_META    (1 << 2)

/* ========================================================================
 * Runtime Structures
 * ======================================================================== */

struct objc_object {
    Class isa;
};

struct objc_selector {
    const char  *_name;
    const char  *_types;
    unsigned int _uid;
};

struct objc_method {
    SEL         name;
    const char *types;
    IMP         imp;
};

struct objc_method_list {
    int                _count;
    int                _space;
    struct objc_method_list *_next;
    struct objc_method  _methods[];
};

struct objc_ivar {
    const char  *_name;
    const char  *_type;
    unsigned int _offset;
};

struct objc_ivar_list {
    int              _count;
    int              _space;
    struct objc_ivar _ivars[];
};

struct objc_property {
    const char *_name;
    const char *_attributes;
};

struct objc_property_list {
    int                  _count;
    int                  _space;
    struct objc_property _properties[];
};

struct objc_protocol_list {
    unsigned int      _count;
    Protocol          _list[];
};

struct objc_protocol {
    const char                *_name;
    struct objc_method_list   *_instanceMethods;
    struct objc_method_list   *_classMethods;
    struct objc_protocol_list *_protocols;
    struct objc_property_list *_properties;
};

struct objc_method_description {
    const char *_name;
    const char *_types;
};

struct objc_class {
    Class                       isa;
    Class                       superclass;
    const char                 *name;
    long                        version;
    long                        instance_size;
    struct objc_method_list    *methodLists;
    struct objc_ivar_list      *ivars;
    struct objc_protocol_list  *protocols;
    struct objc_property_list  *properties;
    unsigned int                info;
};

struct objc_super {
    id    receiver;
    Class super_class;
};

/* ========================================================================
 * Exception Handler Type
 * ======================================================================== */

typedef void (*objc_uncaught_exception_handler)(id);

/* ========================================================================
 * Runtime Function Declarations
 * ======================================================================== */

extern id objc_msgSend(id self, SEL op, ...);
extern id objc_msgSendSuper(struct objc_super *super, SEL op, ...);
extern id objc_msgSend_stret(id self, SEL op, ...);
extern id objc_msgSendSuper_stret(struct objc_super *super, SEL op, ...);

extern SEL sel_registerName(const char *name);
extern SEL sel_getUid(const char *name);
extern const char *sel_getName(SEL sel);
extern BOOL sel_isEqual(SEL lhs, SEL rhs);
extern const char *sel_getTypeEncoding(SEL sel);
extern void sel_setTypeEncoding(SEL sel, const char *types);

extern Class objc_getClass(const char *name);
extern Class objc_lookUpClass(const char *name);
extern Class objc_getMetaClass(const char *name);
extern const char *class_getName(Class cls);
extern Class class_getSuperclass(Class cls);
extern BOOL class_isMetaClass(Class cls);
extern long class_getVersion(Class cls);
extern void class_setVersion(Class cls, long version);
extern size_t class_getInstanceSize(Class cls);

extern Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);
extern void objc_registerClassPair(Class cls);

extern Method class_getInstanceMethod(Class cls, SEL name);
extern Method class_getClassMethod(Class cls, SEL name);
extern Method *class_copyMethodList(Class cls, unsigned int *outCount);
extern BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);
extern void class_replaceMethod(Class cls, SEL name, IMP imp, const char *types);
extern BOOL class_hasMethod(Class cls, SEL name);

extern SEL method_getName(Method m);
extern IMP method_getImplementation(Method m);
extern IMP method_setImplementation(Method m, IMP imp);
extern const char *method_getTypeEncoding(Method m);
extern unsigned int method_getNumberOfArguments(Method m);
extern char *method_getReturnType(Method m, char *buf, size_t bufLen);
extern void method_getArgumentType(Method m, unsigned int index, char *buf, size_t bufLen);
extern id method_invoke(id receiver, Method m, ...);

extern Ivar *class_copyIvarList(Class cls, unsigned int *outCount);
extern const char *ivar_getName(Ivar ivar);
extern const char *ivar_getTypeEncoding(Ivar ivar);
extern ptrdiff_t ivar_getOffset(Ivar ivar);
extern BOOL class_addIvar(Class cls, const char *name, size_t size,
                          unsigned char alignment, const char *types);
extern ptrdiff_t objc_offsetOf(Class cls, const char *name);

extern Property *class_copyPropertyList(Class cls, unsigned int *outCount);
extern Property class_getProperty(Class cls, const char *name);
extern BOOL class_addProperty(Class cls, const char *name, const char *attributes);
extern void class_replaceProperty(Class cls, const char *name, const char *attributes);
extern const char *property_getName(Property prop);
extern const char *property_getAttributes(Property prop);

extern Protocol objc_allocateProtocol(const char *name);
extern void objc_registerProtocol(Protocol proto);
extern Protocol objc_getProtocol(const char *name);
extern void protocol_addMethodDescription(Protocol proto, SEL name,
                                          const char *types, BOOL isRequired,
                                          BOOL isInstanceMethod);
extern void protocol_addProtocol(Protocol proto, Protocol addition);
extern void protocol_addProperty(Protocol proto, const char *name,
                                 const char *attributes, BOOL isRequired,
                                 BOOL isInstanceProperty);
extern const char *protocol_getName(Protocol proto);
extern BOOL protocol_conformsToProtocol(Protocol proto, Protocol other);
extern BOOL protocol_isEqual(Protocol proto, Protocol other);
extern struct objc_method_description *protocol_copyMethodDescriptionList(
    Protocol proto, BOOL isRequiredMethod, BOOL isInstanceMethod,
    unsigned int *outCount);
extern Property *protocol_copyPropertyList(Protocol proto, unsigned int *outCount);
extern Protocol *protocol_copyProtocolList(Protocol proto, unsigned int *outCount);
extern struct objc_method_description protocol_getMethodDescription(
    Protocol proto, SEL sel, BOOL isRequiredMethod, BOOL isInstanceMethod);

extern BOOL class_addProtocol(Class cls, Protocol protocol);
extern BOOL class_conformsToProtocol(Class cls, Protocol protocol);
extern BOOL class_respondsToSelector(Class cls, SEL sel);

extern id class_createInstance(Class cls, size_t extraBytes);
extern id class_createInstanceFromZone(Class cls, size_t extraBytes, void *zone);
extern id object_copy(id obj, size_t extraBytes);
extern id object_dispose(id obj);
extern Class object_getClass(id obj);
extern Class object_setClass(id obj, Class cls);

extern id objc_root_alloc(Class cls, SEL cmd);
extern id objc_root_new(Class cls, SEL cmd);

extern id objc_retain(id obj);
extern void objc_release(id obj);
extern id objc_autorelease(id obj);
extern id objc_autoreleaseReturnValue(id obj);
extern id objc_retainReturnValue(id obj);
extern void *objc_autoreleasePoolPush(void);
extern void objc_autoreleasePoolPop(void *pool);
extern unsigned long objc_retainCount(id obj);

extern void objc_exception_throw(id exception);
extern void objc_exception_try_enter(void *context);
extern void objc_exception_try_exit(void *context);
extern id objc_exception_extract(void *context);
extern BOOL objc_exception_match(Class exceptionType, id exception);
extern objc_uncaught_exception_handler objc_setExceptionHandler(
    objc_uncaught_exception_handler handler);

extern void objc_enumerationMutation(id obj);

extern int _objc_image_count(void);
extern int _objc_getClassList(Class *buffer, int bufferLen);

extern void NSLog(const char *fmt, ...);

#endif /* _OBJC_LIBOBJCRT_H_ */
