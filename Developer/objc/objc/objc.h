/*
 * objc/objc.h - MinSTEP Objective-C Standard Library (Pure C)
 *
 * The root of the MinSTEP Objective-C class hierarchy and basic types.
 * This header is pure C - it can be #included by both the preprocessor
 * and by C code directly.
 *
 * Can be included as:
 *   #include <objc/objc.h>   (standard C include)
 *   #objc                    (preprocessor directive)
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _OBJC_OBJC_H_
#define _OBJC_OBJC_H_

#include "objcrt.h"

/* ========================================================================
 * Foundation Type Aliases (NeXT-style)
 * ======================================================================== */

typedef signed char     SInt8;
typedef unsigned char   UInt8;
typedef signed short    SInt16;
typedef unsigned short  UInt16;
typedef signed int      SInt32;
typedef unsigned int    UInt32;

#if defined(__LP64__) || defined(_WIN64)
typedef signed long     SInt64;
typedef unsigned long   UInt64;
typedef unsigned long   NSUInteger;
typedef long            NSInteger;
#else
typedef signed long long   SInt64;
typedef unsigned long long UInt64;
typedef unsigned int       NSUInteger;
typedef int                NSInteger;
#endif

#ifndef MAXFLOAT
#include <float.h>
#endif

typedef float           CGFloat;

/* ========================================================================
 * Range
 * ======================================================================== */

typedef struct _NSRange {
    NSUInteger location;
    NSUInteger length;
} NSRange;

static inline NSRange NSMakeRange(NSUInteger loc, NSUInteger len)
{
    NSRange r;
    r.location = loc;
    r.length = len;
    return r;
}

#define NSNotFound NSIntegerMax

/* ========================================================================
 * Comparison Result
 * ======================================================================== */

typedef enum {
    NSOrderedAscending = -1,
    NSOrderedSame = 0,
    NSOrderedDescending = 1
} NSComparisonResult;

/* ========================================================================
 * String Encoding Constants
 * ======================================================================== */

#define NSUTF8StringEncoding 4
#define NSASCIIStringEncoding 1
#define NSUnicodeStringEncoding 10
#define NSNEXTSTEPStringEncoding 2
#define NSISOLatin1StringEncoding 5

/* String search options */
#define NSCaseInsensitiveSearch 1
#define NSLiteralSearch 2
#define NSBackwardsSearch 4
#define NSAnchoredSearch 8

/* ========================================================================
 * Error Domains
 * ======================================================================== */

#define NSCocoaErrorDomain "NSCocoaErrorDomain"
#define NSPOSIXErrorDomain "NSPOSIXErrorDomain"

/* ========================================================================
 * Forward Declarations of Foundation-like Classes
 * ======================================================================== */

struct NSString;
struct NSArray;
struct NSDictionary;
struct NSNumber;
struct NSData;
struct NSDate;
struct NSError;
struct NSMutableString;
struct NSMutableArray;
struct NSMutableDictionary;
struct NSMutableData;

/* ========================================================================
 * NSLog
 * ======================================================================== */

extern void NSLog(const char *format, ...);

/* ========================================================================
 * Object - Root Class
 *
 * The root class of the MinSTEP Objective-C hierarchy.
 * All user classes should inherit from Object.
 *
 * C function naming convention:
 *   ClassName_cls_methodName_  - class methods
 *   ClassName_inst_methodName_ - instance methods
 * ======================================================================== */

struct _Object_class_t;

struct Object {
    struct _Object_class_t *isa;
};

/* Allocation and Initialization */
id Object_cls_new(Class cls, SEL _cmd);
id Object_cls_alloc(Class cls, SEL _cmd);
id Object_inst_init(id self, SEL _cmd);
id Object_inst_dealloc(id self, SEL _cmd);

/* Identity and Introspection */
id Object_inst_self(id self, SEL _cmd);
Class Object_inst_class(id self, SEL _cmd);
Class Object_inst_superclass(id self, SEL _cmd);
Class Object_cls_class(Class cls, SEL _cmd);
Class Object_cls_superclass(Class cls, SEL _cmd);

/* Protocol Conformance */
BOOL Object_inst_respondsToSelector_(id self, SEL _cmd, SEL aSelector);
BOOL Object_inst_isKindOfClass_(id self, SEL _cmd, Class aClass);
BOOL Object_inst_isMemberOfClass_(id self, SEL _cmd, Class aClass);
BOOL Object_inst_conformsToProtocol_(id self, SEL _cmd, Protocol aProtocol);

/* Selector Performance */
id Object_inst_performSelector_(id self, SEL _cmd, SEL aSelector);
id Object_inst_performSelector_withObject_(id self, SEL _cmd, SEL aSelector, id object);
id Object_inst_performSelector_withObject_withObject_(id self, SEL _cmd, SEL aSelector, id object1, id object2);

/* Forwarding (NeXT-style) */
void Object_inst_forwardInvocation_(id self, SEL _cmd, id anInvocation);
void Object_inst_doesNotRecognizeSelector_(id self, SEL _cmd, SEL aSelector);
id Object_inst_methodSignatureForSelector_(id self, SEL _cmd, SEL aSelector);

/* Description */
id Object_inst_description(id self, SEL _cmd);
id Object_inst_debugDescription(id self, SEL _cmd);

/* Equality and Hashing */
BOOL Object_inst_isEqual_(id self, SEL _cmd, id object);
unsigned Object_inst_hash(id self, SEL _cmd);

/* Copying */
id Object_inst_copy(id self, SEL _cmd);
id Object_inst_copyWithZone_(id self, SEL _cmd, void *zone);
id Object_inst_mutableCopy(id self, SEL _cmd);
id Object_inst_mutableCopyWithZone_(id self, SEL _cmd, void *zone);

/* Reference Counting */
id Object_inst_retain(id self, SEL _cmd);
void Object_inst_release(id self, SEL _cmd);
id Object_inst_autorelease(id self, SEL _cmd);
unsigned long Object_inst_retainCount(id self, SEL _cmd);

/* Zone Allocation */
id Object_cls_allocWithZone_(Class cls, SEL _cmd, void *zone);

/* Class Metadata */
BOOL Object_cls_instancesRespondToSelector_(Class cls, SEL _cmd, SEL aSelector);
BOOL Object_cls_conformsToProtocol_(Class cls, SEL _cmd, Protocol protocol);
IMP Object_cls_instanceMethodForSelector_(Class cls, SEL _cmd, SEL aSelector);
IMP Object_inst_methodForSelector_(id self, SEL _cmd, SEL aSelector);
unsigned long Object_cls_hash(Class cls, SEL _cmd);
BOOL Object_cls_isSubclassOfClass_(Class cls, SEL _cmd, Class aClass);

/* Error Handling */
void Object_cls_poseAsClass_(Class cls, SEL _cmd, Class aClass);

/* Type Encoding */
const char *Object_inst_objCType(id self, SEL _cmd);

/* ========================================================================
 * NSString - Immutable String
 * ======================================================================== */

struct _NSString_class_t;

struct NSString {
    struct _NSString_class_t *isa;
    struct Object *_super;
};

/* Creation */
id NSString_cls_string(Class cls, SEL _cmd);
id NSString_cls_stringWithString_(Class cls, SEL _cmd, struct NSString *string);
id NSString_cls_stringWithFormat_(Class cls, SEL _cmd, struct NSString *format, ...);
id NSString_cls_stringWithUTF8String_(Class cls, SEL _cmd, const char *nullTerminatedCString);
id NSString_cls_stringWithCString_encoding_(Class cls, SEL _cmd, const char *cString, unsigned long encoding);

/* Initialization */
id NSString_inst_initWithString_(id self, SEL _cmd, struct NSString *string);
id NSString_inst_initWithFormat_(id self, SEL _cmd, struct NSString *format, ...);
id NSString_inst_initWithUTF8String_(id self, SEL _cmd, const char *nullTerminatedCString);
id NSString_inst_initWithCString_encoding_(id self, SEL _cmd, const char *bytes, unsigned long encoding);

/* Length and Content */
unsigned long NSString_inst_length(id self, SEL _cmd);
const char *NSString_inst_UTF8String(id self, SEL _cmd);
const char *NSString_inst_cStringUsingEncoding_(id self, SEL _cmd, unsigned encoding);
BOOL NSString_inst_getCString_maxLength_encoding_(id self, SEL _cmd, char *buffer, unsigned long bufferSize, unsigned encoding);

/* Comparison */
BOOL NSString_inst_isEqualToString_(id self, SEL _cmd, struct NSString *string);
NSComparisonResult NSString_inst_compare_(id self, SEL _cmd, struct NSString *string);
NSComparisonResult NSString_inst_caseInsensitiveCompare_(id self, SEL _cmd, struct NSString *string);
BOOL NSString_inst_hasPrefix_(id self, SEL _cmd, struct NSString *str);
BOOL NSString_inst_hasSuffix_(id self, SEL _cmd, struct NSString *str);
BOOL NSString_inst_containsString_(id self, SEL _cmd, struct NSString *substring);

/* Searching */
unsigned long NSString_inst_rangeOfString_(id self, SEL _cmd, struct NSString *searchString);
unsigned long NSString_inst_rangeOfString_options_(id self, SEL _cmd, struct NSString *searchString, unsigned mask);

/* Substrings */
struct NSString *NSString_inst_substringFromIndex_(id self, SEL _cmd, unsigned long from);
struct NSString *NSString_inst_substringToIndex_(id self, SEL _cmd, unsigned long to);
struct NSString *NSString_inst_substringWithRange_(id self, SEL _cmd, NSRange range);

/* Path Operations */
struct NSString *NSString_inst_lastPathComponent(id self, SEL _cmd);
struct NSString *NSString_inst_stringByDeletingLastPathComponent(id self, SEL _cmd);
struct NSString *NSString_inst_pathExtension(id self, SEL _cmd);
struct NSString *NSString_inst_stringByDeletingPathExtension(id self, SEL _cmd);
struct NSString *NSString_inst_stringByAppendingString_(id self, SEL _cmd, struct NSString *aString);
struct NSString *NSString_inst_stringByAppendingFormat_(id self, SEL _cmd, struct NSString *format, ...);
struct NSString *NSString_inst_stringByAppendingPathComponent_(id self, SEL _cmd, struct NSString *str);
struct NSString *NSString_inst_stringByAppendingPathExtension_(id self, SEL _cmd, struct NSString *ext);
struct NSString *NSString_inst_stringByStandardizingPath(id self, SEL _cmd);
struct NSString *NSString_inst_stringByExpandingTildeInPath(id self, SEL _cmd);

/* Case Conversion */
struct NSString *NSString_inst_uppercaseString(id self, SEL _cmd);
struct NSString *NSString_inst_lowercaseString(id self, SEL _cmd);
struct NSString *NSString_inst_capitalizedString(id self, SEL _cmd);

/* C String Conversion */
unsigned long NSString_inst_maximumLengthOfBytesUsingEncoding_(id self, SEL _cmd, unsigned long enc);
unsigned long NSString_inst_lengthOfBytesUsingEncoding_(id self, SEL _cmd, unsigned long enc);

/* ========================================================================
 * NSMutableString - Mutable String
 * ======================================================================== */

struct _NSMutableString_class_t;

struct NSMutableString {
    struct _NSMutableString_class_t *isa;
    struct NSString *_super;
};

id NSMutableString_cls_stringWithCapacity_(Class cls, SEL _cmd, unsigned long capacity);

void NSMutableString_inst_appendString_(id self, SEL _cmd, struct NSString *aString);
void NSMutableString_inst_appendFormat_(id self, SEL _cmd, struct NSString *format, ...);
void NSMutableString_inst_insertString_atIndex_(id self, SEL _cmd, struct NSString *aString, unsigned long loc);
void NSMutableString_inst_deleteCharactersInRange_(id self, SEL _cmd, NSRange range);
void NSMutableString_inst_replaceCharactersInRange_withString_(id self, SEL _cmd, NSRange range, struct NSString *aString);
void NSMutableString_inst_replaceOccurrencesOfString_withString_options_range_(id self, SEL _cmd, struct NSString *target, struct NSString *replacement, unsigned mask, NSRange rangeOfRangeToSearch);
void NSMutableString_inst_setString_(id self, SEL _cmd, struct NSString *aString);

/* ========================================================================
 * NSArray - Immutable Array
 * ======================================================================== */

struct _NSArray_class_t;

struct NSArray {
    struct _NSArray_class_t *isa;
    struct Object *_super;
};

/* Creation */
id NSArray_cls_array(Class cls, SEL _cmd);
id NSArray_cls_arrayWithObject_(Class cls, SEL _cmd, id anObject);
id NSArray_cls_arrayWithObjects_count_(Class cls, SEL _cmd, const id objects[], unsigned long cnt);

/* Initialization */
id NSArray_inst_initWithObjects_count_(id self, SEL _cmd, const id objects[], unsigned long cnt);

/* Query */
unsigned long NSArray_inst_count(id self, SEL _cmd);
id NSArray_inst_objectAtIndex_(id self, SEL _cmd, unsigned long index);
id NSArray_inst_lastObject(id self, SEL _cmd);
id NSArray_inst_firstObject(id self, SEL _cmd);
BOOL NSArray_inst_containsObject_(id self, SEL _cmd, id anObject);
unsigned long NSArray_inst_indexOfObject_(id self, SEL _cmd, id anObject);

/* Comparing */
BOOL NSArray_inst_isEqualToArray_(id self, SEL _cmd, struct NSArray *otherArray);

/* Enumerating */
void NSArray_inst_makeObjectsPerformSelector_(id self, SEL _cmd, SEL aSelector);
void NSArray_inst_makeObjectsPerformSelector_withObject_(id self, SEL _cmd, SEL aSelector, id object);
id NSArray_inst_objectAtIndexedSubscript_(id self, SEL _cmd, unsigned long idx);

/* Sorting */
struct NSArray *NSArray_inst_sortedArrayUsingSelector_(id self, SEL _cmd, SEL comparator);
struct NSArray *NSArray_inst_sortedArrayUsingFunction_context_(id self, SEL _cmd, NSInteger (*comparator)(id, id, void *), void *context);

/* Strings */
struct NSString *NSString_inst_componentsJoinedByString_(id self, SEL _cmd, struct NSString *separator);

/* ========================================================================
 * NSMutableArray - Mutable Array
 * ======================================================================== */

struct _NSMutableArray_class_t;

struct NSMutableArray {
    struct _NSMutableArray_class_t *isa;
    struct NSArray *_super;
};

id NSMutableArray_cls_arrayWithCapacity_(Class cls, SEL _cmd, unsigned long numItems);

id NSMutableArray_inst_initWithCapacity_(id self, SEL _cmd, unsigned long numItems);

void NSMutableArray_inst_addObject_(id self, SEL _cmd, id anObject);
void NSMutableArray_inst_insertObject_atIndex_(id self, SEL _cmd, id anObject, unsigned long index);
void NSMutableArray_inst_removeLastObject(id self, SEL _cmd);
void NSMutableArray_inst_removeObjectAtIndex_(id self, SEL _cmd, unsigned long index);
void NSMutableArray_inst_removeObject_(id self, SEL _cmd, id anObject);
void NSMutableArray_inst_removeAllObjects(id self, SEL _cmd);
void NSMutableArray_inst_replaceObjectAtIndex_withObject_(id self, SEL _cmd, unsigned long index, id anObject);
void NSMutableArray_inst_setObject_atIndexedSubscript_(id self, SEL _cmd, id obj, unsigned long idx);
void NSMutableArray_inst_sortUsingSelector_(id self, SEL _cmd, SEL comparator);

/* ========================================================================
 * NSDictionary - Immutable Dictionary
 * ======================================================================== */

struct _NSDictionary_class_t;

struct NSDictionary {
    struct _NSDictionary_class_t *isa;
    struct Object *_super;
};

/* Creation */
id NSDictionary_cls_dictionary(Class cls, SEL _cmd);
id NSDictionary_cls_dictionaryWithObject_forKey_(Class cls, SEL _cmd, id object, id key);
id NSDictionary_cls_dictionaryWithObjectsAndKeys_(Class cls, SEL _cmd, id firstObject, ...);
id NSDictionary_cls_dictionaryWithObjects_forKeys_count_(Class cls, SEL _cmd, const id objects[], const id keys[], unsigned long cnt);

/* Initialization */
id NSDictionary_inst_initWithObjectsAndKeys_(id self, SEL _cmd, id firstObject, ...);
id NSDictionary_inst_initWithObjects_forKeys_count_(id self, SEL _cmd, const id objects[], const id keys[], unsigned long cnt);

/* Query */
unsigned long NSDictionary_inst_count(id self, SEL _cmd);
id NSDictionary_inst_objectForKey_(id self, SEL _cmd, id aKey);
struct NSArray *NSDictionary_inst_allKeys(id self, SEL _cmd);
struct NSArray *NSDictionary_inst_allValues(id self, SEL _cmd);
BOOL NSDictionary_inst_isEqualToDictionary_(id self, SEL _cmd, struct NSDictionary *otherDictionary);
BOOL NSDictionary_inst_writeToFile_atomically_(id self, SEL _cmd, struct NSString *path, BOOL useAuxiliaryFile);

/* Object Subscripting (ObjC 2.0) */
id NSDictionary_inst_objectForKeyedSubscript_(id self, SEL _cmd, id key);

/* ========================================================================
 * NSMutableDictionary - Mutable Dictionary
 * ======================================================================== */

struct _NSMutableDictionary_class_t;

struct NSMutableDictionary {
    struct _NSMutableDictionary_class_t *isa;
    struct NSDictionary *_super;
};

id NSMutableDictionary_cls_dictionaryWithCapacity_(Class cls, SEL _cmd, unsigned long numItems);

id NSMutableDictionary_inst_initWithCapacity_(id self, SEL _cmd, unsigned long numItems);

void NSMutableDictionary_inst_removeObjectForKey_(id self, SEL _cmd, id aKey);
void NSMutableDictionary_inst_removeAllObjects(id self, SEL _cmd);
void NSMutableDictionary_inst_setObject_forKey_(id self, SEL _cmd, id anObject, id aKey);
void NSMutableDictionary_inst_setObject_forKeyedSubscript_(id self, SEL _cmd, id obj, id key);

/* ========================================================================
 * NSNumber - Immutable Number
 * ======================================================================== */

struct _NSNumber_class_t;

struct NSNumber {
    struct _NSNumber_class_t *isa;
    struct Object *_super;
};

/* Creation (class methods) */
id NSNumber_cls_numberWithChar_(Class cls, SEL _cmd, char value);
id NSNumber_cls_numberWithUnsignedChar_(Class cls, SEL _cmd, unsigned char value);
id NSNumber_cls_numberWithShort_(Class cls, SEL _cmd, short value);
id NSNumber_cls_numberWithUnsignedShort_(Class cls, SEL _cmd, unsigned short value);
id NSNumber_cls_numberWithInt_(Class cls, SEL _cmd, int value);
id NSNumber_cls_numberWithUnsignedInt_(Class cls, SEL _cmd, unsigned int value);
id NSNumber_cls_numberWithLong_(Class cls, SEL _cmd, long value);
id NSNumber_cls_numberWithUnsignedLong_(Class cls, SEL _cmd, unsigned long value);
id NSNumber_cls_numberWithLongLong_(Class cls, SEL _cmd, long long value);
id NSNumber_cls_numberWithUnsignedLongLong_(Class cls, SEL _cmd, unsigned long long value);
id NSNumber_cls_numberWithFloat_(Class cls, SEL _cmd, float value);
id NSNumber_cls_numberWithDouble_(Class cls, SEL _cmd, double value);
id NSNumber_cls_numberWithBool_(Class cls, SEL _cmd, BOOL value);
id NSNumber_cls_numberWithInteger_(Class cls, SEL _cmd, NSInteger value);
id NSNumber_cls_numberWithUnsignedInteger_(Class cls, SEL _cmd, NSUInteger value);

/* Initialization */
id NSNumber_inst_initWithChar_(id self, SEL _cmd, char value);
id NSNumber_inst_initWithUnsignedChar_(id self, SEL _cmd, unsigned char value);
id NSNumber_inst_initWithShort_(id self, SEL _cmd, short value);
id NSNumber_inst_initWithUnsignedShort_(id self, SEL _cmd, unsigned short value);
id NSNumber_inst_initWithInt_(id self, SEL _cmd, int value);
id NSNumber_inst_initWithUnsignedInt_(id self, SEL _cmd, unsigned int value);
id NSNumber_inst_initWithLong_(id self, SEL _cmd, long value);
id NSNumber_inst_initWithUnsignedLong_(id self, SEL _cmd, unsigned long value);
id NSNumber_inst_initWithLongLong_(id self, SEL _cmd, long long value);
id NSNumber_inst_initWithUnsignedLongLong_(id self, SEL _cmd, unsigned long long value);
id NSNumber_inst_initWithFloat_(id self, SEL _cmd, float value);
id NSNumber_inst_initWithDouble_(id self, SEL _cmd, double value);
id NSNumber_inst_initWithBool_(id self, SEL _cmd, BOOL value);
id NSNumber_inst_initWithInteger_(id self, SEL _cmd, NSInteger value);
id NSNumber_inst_initWithUnsignedInteger_(id self, SEL _cmd, NSUInteger value);

/* Accessor Methods */
char NSNumber_inst_charValue(id self, SEL _cmd);
unsigned char NSNumber_inst_unsignedCharValue(id self, SEL _cmd);
short NSNumber_inst_shortValue(id self, SEL _cmd);
unsigned short NSNumber_inst_unsignedShortValue(id self, SEL _cmd);
int NSNumber_inst_intValue(id self, SEL _cmd);
unsigned int NSNumber_inst_unsignedIntValue(id self, SEL _cmd);
long NSNumber_inst_longValue(id self, SEL _cmd);
unsigned long NSNumber_inst_unsignedLongValue(id self, SEL _cmd);
long long NSNumber_inst_longLongValue(id self, SEL _cmd);
unsigned long long NSNumber_inst_unsignedLongLongValue(id self, SEL _cmd);
float NSNumber_inst_floatValue(id self, SEL _cmd);
double NSNumber_inst_doubleValue(id self, SEL _cmd);
BOOL NSNumber_inst_boolValue(id self, SEL _cmd);
NSInteger NSNumber_inst_integerValue(id self, SEL _cmd);
NSUInteger NSNumber_inst_unsignedIntegerValue(id self, SEL _cmd);

/* String Representation */
struct NSString *NSNumber_inst_stringValue(id self, SEL _cmd);
struct NSString *NSNumber_inst_description(id self, SEL _cmd);

/* Comparison */
NSComparisonResult NSNumber_inst_compare_(id self, SEL _cmd, struct NSNumber *otherNumber);
BOOL NSNumber_inst_isEqualToNumber_(id self, SEL _cmd, struct NSNumber *number);

/* Numeric Type Identification */
const char *NSNumber_inst_objCType(id self, SEL _cmd);

/* ========================================================================
 * NSData - Immutable Data
 * ======================================================================== */

struct _NSData_class_t;

struct NSData {
    struct _NSData_class_t *isa;
    struct Object *_super;
};

/* Creation */
id NSData_cls_data(Class cls, SEL _cmd);
id NSData_cls_dataWithBytes_length_(Class cls, SEL _cmd, const void *bytes, unsigned long length);
id NSData_cls_dataWithContentsOfFile_(Class cls, SEL _cmd, struct NSString *path);

/* Initialization */
id NSData_inst_initWithBytes_length_(id self, SEL _cmd, const void *bytes, unsigned long length);
id NSData_inst_initWithContentsOfFile_(id self, SEL _cmd, struct NSString *path);

/* Access */
const void *NSData_inst_bytes(id self, SEL _cmd);
unsigned long NSData_inst_length(id self, SEL _cmd);
void NSData_inst_getBytes_length_(id self, SEL _cmd, void *buffer, unsigned long length);

/* Writing */
BOOL NSData_inst_writeToFile_atomically_(id self, SEL _cmd, struct NSString *path, BOOL useAuxiliaryFile);

/* ========================================================================
 * NSMutableData - Mutable Data
 * ======================================================================== */

struct _NSMutableData_class_t;

struct NSMutableData {
    struct _NSMutableData_class_t *isa;
    struct NSData *_super;
};

id NSMutableData_cls_dataWithCapacity_(Class cls, SEL _cmd, unsigned long aNumItems);
id NSMutableData_cls_dataWithLength_(Class cls, SEL _cmd, unsigned long length);

id NSMutableData_inst_initWithCapacity_(id self, SEL _cmd, unsigned long capacity);
id NSMutableData_inst_initWithLength_(id self, SEL _cmd, unsigned long length);

void *NSMutableData_inst_mutableBytes(id self, SEL _cmd);
void NSMutableData_inst_setLength_(id self, SEL _cmd, unsigned long length);

void NSMutableData_inst_appendData_(id self, SEL _cmd, id other);
void NSMutableData_inst_appendBytes_length_(id self, SEL _cmd, const void *bytes, unsigned long length);
void NSMutableData_inst_replaceBytesInRange_withBytes_(id self, SEL _cmd, NSRange range, const void *bytes);

/* ========================================================================
 * NSDate - Immutable Date
 * ======================================================================== */

struct _NSDate_class_t;

struct NSDate {
    struct _NSDate_class_t *isa;
    struct Object *_super;
};

/* Creation */
id NSDate_cls_date(Class cls, SEL _cmd);
id NSDate_cls_dateWithTimeIntervalSinceNow_(Class cls, SEL _cmd, double secs);
id NSDate_cls_dateWithTimeIntervalSince1970_(Class cls, SEL _cmd, double secs);

/* Initialization */
id NSDate_inst_initWithTimeIntervalSinceNow_(id self, SEL _cmd, double secs);
id NSDate_inst_initWithTimeIntervalSince1970_(id self, SEL _cmd, double secs);
id NSDate_inst_initWithTimeInterval_sinceDate_(id self, SEL _cmd, double secs, id aDate);

/* Access */
double NSDate_inst_timeIntervalSince1970(id self, SEL _cmd);
double NSDate_inst_timeIntervalSinceNow(id self, SEL _cmd);
double NSDate_inst_timeIntervalSinceDate_(id self, SEL _cmd, id anotherDate);

/* Comparison */
BOOL NSDate_inst_isEqualToDate_(id self, SEL _cmd, struct NSDate *otherDate);
NSComparisonResult NSDate_inst_compare_(id self, SEL _cmd, struct NSDate *anotherDate);
BOOL NSDate_inst_earlierThan_(id self, SEL _cmd, struct NSDate *anotherDate);
BOOL NSDate_inst_laterThan_(id self, SEL _cmd, struct NSDate *anotherDate);
BOOL NSDate_inst_isDateEqualToDate_(id self, SEL _cmd, struct NSDate *otherDate);

/* Description */
id NSDate_inst_description(id self, SEL _cmd);

/* ========================================================================
 * NSError
 * ======================================================================== */

struct _NSError_class_t;

struct NSError {
    struct _NSError_class_t *isa;
    struct Object *_super;
};

id NSError_cls_errorWithDomain_code_userInfo_(Class cls, SEL _cmd, struct NSString *domain, NSInteger code, struct NSDictionary *dict);

id NSError_inst_initWithDomain_code_userInfo_(id self, SEL _cmd, struct NSString *domain, NSInteger code, struct NSDictionary *dict);

struct NSString *NSError_inst_domain(id self, SEL _cmd);
NSInteger NSError_inst_code(id self, SEL _cmd);
struct NSDictionary *NSError_inst_userInfo(id self, SEL _cmd);
struct NSString *NSError_inst_localizedDescription(id self, SEL _cmd);
struct NSString *NSError_inst_localizedFailureReason(id self, SEL _cmd);
struct NSString *NSError_inst_localizedRecoverySuggestion(id self, SEL _cmd);

/* ========================================================================
 * Convenience Macros (NeXT/Apple-style)
 * ======================================================================== */

/* Autorelease Pool (NeXT-style) */
#define NSAutoreleasePool() \
    do { \
        void *_pool = objc_autoreleasePoolPush()

#define NS_ENDPOOL \
        objc_autoreleasePoolPop(_pool); \
    } while(0)

/* String Constants */
#define NSLocalizedString(key, comment) ((id)(key))
#define NSLocalizedStringFromTable(key, table, comment) ((id)(key))
#define NSLocalizedStringFromTableInBundle(key, table, bundle, comment) ((id)(key))

/* ========================================================================
 * Property Support Macros (Objective-C 2.0)
 * ======================================================================== */

#define OBJC_PROPERTY_GETTER(type, name) \
    -(type)name

#define OBJC_PROPERTY_SETTER(type, name) \
    -(void)set##name:(type)value

#define OBJC_PROPERTY_IVAR(name) _##name

#define OBJC_DYNAMIC_ATTRIBUTE "D"

/* ========================================================================
 * Fast Enumeration Support
 * ======================================================================== */

#define OBJC_ENUMERATIONMutationHandler objc_enumerationMutation

/* ========================================================================
 * Compatibility Macros
 * ======================================================================== */

#ifndef OBJC_OLD_DISPATCH_PROTOTYPE
#define OBJC_OLD_DISPATCH_PROTOTYPE 0
#endif

/* ARC support */
#if OBJC_ARC
  #define OBJC_RETAIN(x) (x)
  #define OBJC_RELEASE(x)
  #define OBJC_AUTORELEASE(x) (x)
  #define OBJC_SUPER_DEALLOC
#else
  #define OBJC_RETAIN(x) objc_retain(x)
  #define OBJC_RELEASE(x) objc_release(x)
  #define OBJC_AUTORELEASE(x) objc_autorelease(x)
  #define OBJC_SUPER_DEALLOC [super dealloc]
#endif

/* Property attribute strings */
#define OBJC_PROPERTY_READONLY_STR    "R"
#define OBJC_PROPERTY_READWRITE_STR   "T,N,V_%s"
#define OBJC_PROPERTY_COPY_STR        "C"
#define OBJC_PROPERTY_RETAIN_STR      "&"
#define OBJC_PROPERTY_NONATOMIC_STR   "N"
#define OBJC_PROPERTY_GETTER_STR(s)   "G" s
#define OBJC_PROPERTY_SETTER_STR(s)   "S" s

/* ========================================================================
 * Encoding Constants (for @encode())
 * ======================================================================== */

#define _OBJC_INTENC_INT        "i"
#define _OBJC_INTENC_UINT       "I"
#define _OBJC_INTENC_LONG       "l"
#define _OBJC_INTENC_ULONG      "L"
#define _OBJC_INTENC_LONGLONG   "q"
#define _OBJC_INTENC_ULONGLONG  "Q"
#define _OBJC_INTENC_FLOAT      "f"
#define _OBJC_INTENC_DOUBLE     "d"
#define _OBJC_INTENC_CHAR       "c"
#define _OBJC_INTENC_UCHAR      "C"
#define _OBJC_INTENC_SHORT      "s"
#define _OBJC_INTENC_USHORT     "S"
#define _OBJC_INTENC_BOOL       "B"
#define _OBJC_INTENC_CHARPTR    "*"
#define _OBJC_INTENC_ID         "@"
#define _OBJC_INTENC_CLASS      "#"
#define _OBJC_INTENC_SEL        ":"
#define _OBJC_INTENC_VOID       "v"

/* ========================================================================
 * Block Support Stubs
 * ======================================================================== */

/* Block syntax (^) requires Clang. For GCC, provide a basic stub. */
#ifdef __clang__
  typedef void (^dispatch_block_t)(void);
#else
  typedef void (*dispatch_block_t)(void);
#endif

/* ========================================================================
 * ARC-compatible Memory Management Macros
 * ======================================================================== */

#if OBJC_ARC
  #define OBJC_RETAIN_PROPERTY(p) (p)
  #define OBJC_RELEASE_PROPERTY(p)
  #define OBJC_AUTORELEASE_PROPERTY(p) (p)
  #define OBJC_COPY_PROPERTY(p) (p)
  #define OBJC_ASSIGN_PROPERTY(p) (p)
  #define OBJC_WEAK_PROPERTY
  #define OBJC_STRONG_PROPERTY
  #define OBJC_UNSAFE_UNRETAINED_PROPERTY __unsafe_unretained
#else
  #define OBJC_RETAIN_PROPERTY(p) objc_retain(p)
  #define OBJC_RELEASE_PROPERTY(p) objc_release(p)
  #define OBJC_AUTORELEASE_PROPERTY(p) objc_autorelease(p)
  #define OBJC_COPY_PROPERTY(p) [p copy]
  #define OBJC_ASSIGN_PROPERTY(p) (p)
  #define OBJC_WEAK_PROPERTY
  #define OBJC_STRONG_PROPERTY
  #define OBJC_UNSAFE_UNRETAINED_PROPERTY
#endif

#endif /* _OBJC_OBJC_H_ */
