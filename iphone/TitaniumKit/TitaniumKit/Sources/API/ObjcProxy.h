/**
 * Appcelerator Titanium Mobile
 * Copyright (c) 2018-Present by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License
 * Please see the LICENSE included with this distribution for details.
 */
@import JavaScriptCore;
#import <pthread.h>

// Macros to make life easier for defining properties with getters/setter accessor methods (which we'll remove in SDK 9.0.0

// Defines a setProp() accessor method in JS-world that points to setterProp:(TYPE)value in native code
#define SETTER(TYPE, NAME) JSExportAs(set##NAME, -(void)setter##NAME \
                                      : (TYPE)value);
// Implements the setterProp:(TYPEvalue method that forwards to typical setProp:value impl
#define SETTER_IMPL(TYPE, NAME)     \
  -(void)setter##NAME : (TYPE)value \
  {                                 \
    [self set##NAME:value];         \
  }

// Defines a getProp() method in JS-land
#define GETTER(TYPE, NAME) \
  -(TYPE)get##NAME __attribute((deprecated("Use the property instead.")));
// Implements the special getter method that forwards to typical property name getter
#define GETTER_IMPL(TYPE, LOWER, UPPER) \
  -(TYPE)get##UPPER                     \
  {                                     \
    return [self LOWER];                \
  }

// Defines a Read/write pair of accessors
#define READWRITE(TYPE, NAME) \
  GETTER(TYPE, NAME);         \
  SETTER(TYPE, NAME);

#define READWRITE_IMPL(TYPE, LOWER, UPPER) \
  GETTER_IMPL(TYPE, LOWER, UPPER);         \
  SETTER_IMPL(TYPE, UPPER);

#define PROPERTY(TYPE, LOWER, UPPER) \
  @property TYPE LOWER;              \
  READWRITE(TYPE, UPPER);

#define READONLY_PROPERTY(TYPE, LOWER, UPPER) \
  @property TYPE LOWER;                       \
  GETTER(TYPE, UPPER);

#define CONSTANT(TYPE, NAME) \
  @property (readonly) TYPE NAME;

@protocol ProxyExports <JSExport>

// Properties (and accessors)
READONLY_PROPERTY(NSString *, apiName, ApiName);

/**
 Indicates that this proxy should honor bubbling of user events, if the proxy
 is the type that has a parent to bubble to (This is primairly views, but may
 have some exceptions).
 */
PROPERTY(BOOL, bubbleParent, BubbleParent);

// Methods
JSExportAs(addEventListener,
           -(void)addEventListener
           : (NSString *)name withCallback
           : (JSValue *)callback);
JSExportAs(removeEventListener,
           -(void)removeEventListener
           : (NSString *)name withCallback
           : (JSValue *)callback);
JSExportAs(fireEvent,
           -(void)fireEvent
           : (NSString *)name withDict
           : (NSDictionary *)dict);

@end

/**
 The base class for Titanium proxies using new Obj-C API.
 */
@interface ObjcProxy : NSObject <ProxyExports> {
  @private
  NSMutableDictionary *m_listeners; // new listener map for Obj-C JSC API
  pthread_rwlock_t m_listenerLock;
  NSURL *baseURL;
}

- (NSURL *)_baseURL;

// hooks for when an event listener gets added/removed
- (void)_listenerAdded:(NSString *)type count:(int)count;
- (void)_listenerRemoved:(NSString *)type count:(int)count;

/**
 Whether or not the proxy has listeners for the specified event type.
 @param type The event type.
 @return _YES_ if the proxy has any listeners for the specified event type, _NO_ otherwise.
 */
- (BOOL)_hasListeners:(NSString *)type;

- (void)fireEvent:(NSString *)name withDict:(NSDictionary *)dict;
/**
 Tells the proxy to fire an event of the specified type to a listener.
 @param type The event type.
 @param obj The event properties.
 @param listener The listener to fire event for.
 */
- (void)_fireEventToListener:(NSString *)type withObject:(id)obj listener:(JSValue *)listener;

/**
 The convenience method to raise an exception .
 @param reason The exception reason.
 @param subreason The exception subreason.
 @param location The exception location.
 */
+ (void)throwException:(NSString *)reason subreason:(NSString *)subreason location:(NSString *)location;
- (void)throwException:(NSString *)reason subreason:(NSString *)subreason location:(NSString *)location;

// FIXME: Should id be TiProxy* here?
- (id)JSValueToNative:(JSValue *)jsValue;
- (JSValue *)NativeToJSValue:(id)proxy;
@end
