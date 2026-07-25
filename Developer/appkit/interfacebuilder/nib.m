/*
 * appkit/interfacebuilder/nib.m - MinSTEP NIB Builder Implementation
 *
 * Implementation of MXNibBuilder for loading .nib.m files and
 * instantiating objects defined in them.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#objc
#import <appkit/interfacebuilder/nib.h>

/* ========================================================================
 * MXNibBuilder Implementation
 * ======================================================================== */

@implementation MXNibBuilder

+ (id)new
{
    MXNibBuilder *obj = [super new];
    if (obj) {
        obj->owner = nil;
        obj->NXWindow = nil;
        obj->NXText = nil;
        obj->NXButton = nil;
        obj->NXScrollView = nil;
        obj->NXMenu = nil;
        obj->NXMenuItem = nil;
        obj->NXOpenPanel = nil;
        obj->NXSavePanel = nil;
        obj->NXAlertPanel = nil;
        obj->NXControl = nil;
        obj->NXView = nil;
    }
    return obj;
}

- (void)loadNibFile:(const char *)nibPath withOwner:(id)anOwner
{
    owner = anOwner;
    
    /* In a full implementation, this would:
     * 1. Read the .nib.m file
     * 2. Execute the MXNibBuilder subclass's declareInterfaceWithOwner:
     * 3. Perform connections
     */
    
    /* For now, call the subclass method if overridden */
    if ([self respondsToSelector:@selector(declareInterfaceWithOwner:)]) {
        [self declareInterfaceWithOwner:owner];
    }
}

- (void)declareInterfaceWithOwner:(id)ownerObj
{
    /* Subclasses implement this to declare interface elements */
}

- (void)connectOutlet:(const char *)outletName ofOwner:(id)anOwner withValue:(id)value
{
    /* Set the ivar of the owner to the value
     * This uses runtime introspection to find and set the ivar
     */
    Class ownerClass = [anOwner class];
    Ivar ivar = class_getInstanceVariable(ownerClass, outletName);
    if (ivar) {
        object_setIvar(anOwner, ivar, value);
    }
}

- (id)classname
{
    /* Returns an instance of the class based on the calling context
     * The actual class name is determined from the message name
     */
    return nil;
}

- (id)nil_sentinel
{
    return nil;
}

@end

/* ========================================================================
 * MXNibDecoder Implementation
 * ======================================================================== */

@implementation MXNibDecoder

+ (id)decoderWithData:(const char *)nibData length:(int)length
{
    MXNibDecoder *decoder = [[self alloc] initWithData:nibData length:length];
    return [decoder autorelease];
}

- (id)initWithData:(const char *)nibData length:(int)length
{
    self = [super init];
    if (self) {
        data = nibData;
        dataLength = length;
        position = 0;
    }
    return self;
}

- (id)decodeObject
{
    /* Object decoding would parse object references */
    return nil;
}

- (const char *)decodeString
{
    if (position >= dataLength) return NULL;
    
    /* Skip whitespace */
    while (position < dataLength && (data[position] == ' ' || 
                                     data[position] == '\t' ||
                                     data[position] == '\n')) {
        position++;
    }
    
    if (position >= dataLength) return NULL;
    
    int start = position;
    while (position < dataLength && data[position] != ',' && 
           data[position] != ')' && data[position] != ';' &&
           data[position] != '\n') {
        position++;
    }
    
    int len = position - start;
    char *result = (char *)malloc(len + 1);
    strncpy(result, data + start, len);
    result[len] = '\0';
    
    return result;
}

- (int)decodeInt
{
    if (position >= dataLength) return 0;
    
    int result = 0;
    BOOL negative = NO;
    
    /* Skip whitespace */
    while (position < dataLength && (data[position] == ' ' || 
                                     data[position] == '\t')) {
        position++;
    }
    
    if (position < dataLength && data[position] == '-') {
        negative = YES;
        position++;
    }
    
    while (position < dataLength && data[position] >= '0' && 
           data[position] <= '9') {
        result = result * 10 + (data[position] - '0');
        position++;
    }
    
    return negative ? -result : result;
}

- (float)decodeFloat
{
    /* Simple float decoding */
    if (position >= dataLength) return 0.0f;
    
    /* Skip whitespace */
    while (position < dataLength && (data[position] == ' ' || 
                                     data[position] == '\t')) {
        position++;
    }
    
    float result = 0.0f;
    float fraction = 0.0f;
    float divisor = 1.0f;
    BOOL afterDecimal = NO;
    BOOL negative = NO;
    
    if (position < dataLength && data[position] == '-') {
        negative = YES;
        position++;
    }
    
    while (position < dataLength) {
        if (data[position] >= '0' && data[position] <= '9') {
            if (afterDecimal) {
                divisor *= 10.0f;
                fraction = fraction * 10 + (data[position] - '0');
            } else {
                result = result * 10 + (data[position] - '0');
            }
        } else if (data[position] == '.') {
            afterDecimal = YES;
        } else {
            break;
        }
        position++;
    }
    
    result = result + fraction / divisor;
    return negative ? -result : result;
}

- (BOOL)decodeBool
{
    if (position >= dataLength) return NO;
    
    /* Skip whitespace */
    while (position < dataLength && (data[position] == ' ' || 
                                     data[position] == '\t')) {
        position++;
    }
    
    /* Check for YES/NO or 1/0 */
    if (position + 3 <= dataLength && strncmp(data + position, "YES", 3) == 0) {
        position += 3;
        return YES;
    } else if (position + 2 <= dataLength && strncmp(data + position, "NO", 2) == 0) {
        position += 2;
        return NO;
    }
    
    int val = [self decodeInt];
    return val != 0;
}

- (BOOL)isAtEnd
{
    return position >= dataLength;
}

@end

/* ========================================================================
 * MXNibEncoder Implementation
 * ======================================================================== */

@implementation MXNibEncoder

+ (id)encoder
{
    MXNibEncoder *encoder = [[self alloc] init];
    return [encoder autorelease];
}

- (id)init
{
    self = [super init];
    if (self) {
        dataCapacity = 4096;
        dataLength = 0;
        data = (char *)malloc(dataCapacity);
        data[0] = '\0';
    }
    return self;
}

- (void)dealloc
{
    if (data) free(data);
    [super dealloc];
}

- (void)ensureCapacity:(int)needed
{
    while (dataLength + needed >= dataCapacity) {
        dataCapacity *= 2;
        data = (char *)realloc(data, dataCapacity);
    }
}

- (void)encodeObject:(id)object
{
    /* Object encoding would write object references */
    char buf[256];
    snprintf(buf, sizeof(buf), "@%p", object);
    [self encodeString:buf];
}

- (void)encodeString:(const char *)string
{
    if (!string) {
        string = "";
    }
    int len = strlen(string);
    [self ensureCapacity:len + 4];
    
    data[dataLength++] = '"';
    strcpy(data + dataLength, string);
    dataLength += len;
    data[dataLength++] = '"';
    data[dataLength++] = ',';
    data[dataLength] = '\0';
}

- (void)encodeInt:(int)value
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d,", value);
    int len = strlen(buf);
    [self ensureCapacity:len];
    strcpy(data + dataLength, buf);
    dataLength += len;
}

- (void)encodeFloat:(float)value
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%g,", value);
    int len = strlen(buf);
    [self ensureCapacity:len];
    strcpy(data + dataLength, buf);
    dataLength += len;
}

- (void)encodeBool:(BOOL)value
{
    const char *str = value ? "YES," : "NO,";
    int len = strlen(str);
    [self ensureCapacity:len];
    strcpy(data + dataLength, str);
    dataLength += len;
}

- (const char *)data
{
    return data;
}

- (int)dataLength
{
    return dataLength;
}

@end