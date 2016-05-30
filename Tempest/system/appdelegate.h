#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#include <Tempest/Platform>

#ifdef __OSX__
#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
  @property (strong, nonatomic) NSWindow        *window;
  @property (strong, nonatomic) NSOpenGLContext *openGLContext;
  @property (strong, nonatomic) NSTimer         *timer;
@end

#endif

#endif // APPDELEGATE_H
