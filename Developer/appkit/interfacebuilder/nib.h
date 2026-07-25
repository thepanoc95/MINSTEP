/*
 * appkit/interfacebuilder/nib.h - MinSTEP NIB Builder Header
 *
 * MXNibBuilder provides the interface for loading .nib.m files
 * (MinSTEP's text-based NIB format) and instantiating objects.
 *
 * Copyright (c) 2026 MinSTEP Project
 * Licensed under the MIT License.
 */

#ifndef _APPKIT_INTERFACEBUILDER_NIB_H_
#define _APPKIT_INTERFACEBUILDER_NIB_H_

#objc
#import <appkit/appkit.h>

/* NIB loading options */
#define NX_NIBLocalizableImagedictKey     "NIBLocalizableImagedict"
#define NX_NIBImportedClassdictKey        "NIBImportedClassdict"
#define NX_NIBClassVersionsKey            "NIBClassVersions"
#define NX_NIBVariablesKey                "NIBVariables"

/* Global declarations used in .nib.m files */
#define DECLARE(classname) [self classname]
#define OUTLET(owner, outletname, value) [self connectOutlet:outletname ofOwner:owner withValue:value]
#define ACTION(owner, actionname, selector) [self registerAction:selector ofOwner:owner withName:actionname]
#define CONNECT(owner1, outlet, owner2, ivarname) [self connectOutlet:ivarname ofOwner:owner1 toOutlet:outlet ofOwner:owner2]
#define SUPER [super superclass]
#define nil_sentinel() [self nil_sentinel]

/* ========================================================================
 * MXNibBuilder - NIB File Builder/Loader
 * ======================================================================== */

@interface MXNibBuilder : Object
{
    id owner;
    NXWindow *NXWindow;
    NXText *NXText;
    NXButton *NXButton;
    NXScrollView *NXScrollView;
    NXMenu *NXMenu;
    NXMenuItem *NXMenuItem;
    NXOpenPanel *NXOpenPanel;
    NXSavePanel *NXSavePanel;
    NXAlertPanel *NXAlertPanel;
    NXControl *NXControl;
    NXView *NXView;
}

+ (id)new;

- (void)loadNibFile:(const char *)nibPath withOwner:(id)anOwner;
- (void)declareInterfaceWithOwner:(id)owner;
- (void)connectOutlet:(const char *)outletName ofOwner:(id)owner withValue:(id)value;
- (id)classname;
- (id)nil_sentinel;

@end

/* ========================================================================
 * MXNibDecoder - NIB File Decoder
 * ======================================================================== */

@interface MXNibDecoder : Object
{
    const char *data;
    int dataLength;
    int position;
}

+ (id)decoderWithData:(const char *)data length:(int)length;
- (id)initWithData:(const char *)data length:(int)length;
- (id)decodeObject;
- (const char *)decodeString;
- (int)decodeInt;
- (float)decodeFloat;
- (BOOL)decodeBool;
- (BOOL)isAtEnd;

@end

/* ========================================================================
 * MXNibEncoder - NIB File Encoder
 * ======================================================================== */

@interface MXNibEncoder : Object
{
    char *data;
    int dataLength;
    int dataCapacity;
}

+ (id)encoder;
- (id)init;
- (void)encodeObject:(id)object;
- (void)encodeString:(const char *)string;
- (void)encodeInt:(int)value;
- (void)encodeFloat:(float)value;
- (void)encodeBool:(BOOL)value;
- (const char *)data;
- (int)dataLength;

@end

#endif /* _APPKIT_INTERFACEBUILDER_NIB_H_ */