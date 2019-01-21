
#include <cstdarg> // va_start
#include <cstdio> // printf, snprintf, vsnprintf
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
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

#import <simd/simd.h> // TODO: Remove.

#if defined(DEBUG)
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#include "Math.h"
#include "Core_iOS.h"
#include "Core.h"
#include "Data.h"
#include "Formats.h"
#include "Build.h"
#include "Build_Metal.h"

int main(int argc, const char * argv[]) {
    Builder builder;
    return 0;
}


