#ifndef APPDELEGATE_H
#define APPDELEGATE_H

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
  @property (strong, nonatomic) NSWindow        *window;
  @property (strong, nonatomic) NSOpenGLContext *openGLContext;
@end

#endif // APPDELEGATE_H
