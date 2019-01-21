
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#define _HAS_EXCEPTIONS 0

#include <crtdbg.h>
#include <cstdio> // printf, snprintf, vsnprintf
#include <cstdlib> // srand
#include <D3Dcompiler.h>
#include <math.h> // sinf, cosf
#include <Windows.h>
#include <wrl.h>

#pragma comment(lib, "d3dcompiler.lib")

#if defined(DEBUG)
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#include "Math.h"
#include "Core_Windows.h"
#include "Core.h"
#include "Data.h"
#include "Formats.h"
#include "Build.h"
#include "Build_Windows.h"

int main(int argc, char** argv) {
    {
        Builder builder;
        Packager packager;
    }
    DEBUG_ONLY(_CrtDumpMemoryLeaks();)
    return 0;
}
