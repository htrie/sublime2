
#include <cstdarg> // va_start
#include <cstdio> // printf, snprintf, vsnprintf
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libkern/OSAtomic.h> // OSAtomicXXX
#include <math.h> // sinf, cosf
#include <new> // placement new
#include <pthread.h> // pthread_xxx
#include <string.h> // memcpy, memset
#include <sys/mman.h>
#include <sys/sysctl.h> // sysctlbyname
#include <sys/stat.h>
#include <sys/time.h> // gettimeofday
#include <sys/types.h>
#include <unistd.h> // usleep

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <ModelIO/ModelIO.h> // TODO: Remove.

#if defined(DEBUG)
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#if !defined(DEBUG)
#define RELEASE_ONLY(A) A
#else
#define RELEASE_ONLY(A)
#endif

#include "Math.h"
#include "Interface.h"
#include "Core_iOS.h"
#include "Core.h"
#include "Data.h"
#include "Formats.h"
#include "Render_Metal.h"
#include "Debug_Metal.h"
#include "Debug.h"
#include "Audio_iOS.h"
#include "Control_iOS.h"
#include "Engine.h"

@interface Application : UIViewController <UIApplicationDelegate, MTKViewDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation Application {
    Engine engine;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    DEBUG_ONLY(Log::Put("didFinishLaunchingWithOptions\n");)
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    DEBUG_ONLY(Log::Put("applicationWillResignActive\n");)
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    DEBUG_ONLY(Log::Put("applicationDidEnterBackground\n");)
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    DEBUG_ONLY(Log::Put("applicationWillEnterForeground\n");)
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    DEBUG_ONLY(Log::Put("applicationDidBecomeActive\n");)
}

- (void)applicationWillTerminate:(UIApplication *)application {
    DEBUG_ONLY(Log::Put("applicationWillTerminate\n");)
}

- (void)viewDidLoad {
    [super viewDidLoad];
    RootPath = File::ExtractPath(String([[[NSBundle mainBundle] pathForResource:@"Info" ofType:@"plist"] UTF8String]));
    new (&engine) Engine(Parameters(self.view.bounds.size.width, self.view.bounds.size.height, self.view));
    ((MTKView*)self.view).delegate = self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    engine.Update();
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    DEBUG_ONLY(Log::Put("drawableSizeWillChange %d x %d\n", size.width, size.height);)
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([Application class]));
    }
}
