
#define NOMINMAX
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#define _HAS_EXCEPTIONS 0

#include <crtdbg.h>
#include <cstdio> // printf, snprintf, vsnprintf
#include <cstdlib> // srand
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <dxgi1_5.h>
#include <DxgiDebug.h>
#include <math.h> // sinf, cosf
#include <Windows.h>
#include <wrl.h>
#include <Xaudio2.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Xaudio2.lib")

#if defined(DEBUG)
#define DEBUG_ONLY(A) A
#else
#define DEBUG_ONLY(A)
#endif

#include "Math.h"
#include "Interface.h"
#include "Core_Windows.h"
#include "Core.h"
#include "Data.h"
#include "Audio_XAudio2.h"
#include "Control_Windows.h"
#include "Render_DirectX12.h"
#include "Debug_DirectX12.h"
#include "Debug.h"
#include "Engine.h"


class Application : public NoCopy {
    Engine engine;

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Application* app = Create(hWnd, message, wParam, lParam);
        if (!app->ProcessMessage(hWnd, message, wParam, lParam))
            return DefWindowProc(hWnd, message, wParam, lParam);
        return 0;
    }

    static Application* Create(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_CREATE) {
            LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
            return 0;
        }
        return reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    bool ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_SIZE: return true;
        case WM_PAINT: engine.Update(); return true;
        case WM_CLOSE:
        case WM_DESTROY: PostQuitMessage(0); return true;
        case WM_LBUTTONDOWN: Mouse(wParam, lParam, MousePhase::Down, Button::Left); return true;
        case WM_LBUTTONUP: Mouse(wParam, lParam, MousePhase::Up, Button::Left); return true;
        case WM_MBUTTONDOWN: Mouse(wParam, lParam, MousePhase::Down, Button::Middle); return true;
        case WM_MBUTTONUP: Mouse(wParam, lParam, MousePhase::Up, Button::Middle); return true;
        case WM_RBUTTONDOWN: Mouse(wParam, lParam, MousePhase::Down, Button::Right); return true;
        case WM_RBUTTONUP: Mouse(wParam, lParam, MousePhase::Up, Button::Right); return true;
        case WM_MOUSEWHEEL: Mouse(wParam, lParam, MousePhase::Scroll, Button::Wheel); return true;
        case WM_MOUSEMOVE: Mouse(wParam, lParam, MousePhase::Dragged, Button::None); return true;
        case WM_KEYDOWN: Keyboard(wParam, lParam, KeyboardPhase::Pressed); return true;
        case WM_KEYUP: Keyboard(wParam, lParam, KeyboardPhase::Released); return true;
        case WM_CHAR: KeyboardChar(wParam, lParam, KeyboardPhase::Char); return true;
        }
        return false;
    }

    void Mouse(WPARAM wParam, LPARAM lParam, MousePhase phase, Button button) {
        const float u = ((float)(short)LOWORD(lParam));
        const float v = ((float)(short)HIWORD(lParam));
        const float w = ((float)(short)HIWORD(wParam));
        engine.Mouse(phase, button, u, v, w);
    }

    void Keyboard(WPARAM wParam, LPARAM lParam, KeyboardPhase phase) {
        Key key = Key::None;
        if ((wParam >= VK_F1) && (wParam <= VK_F12)) {
            key = (Key)((unsigned)Key::F1 + wParam - VK_F1);
        } else if ((wParam >= 'A') && (wParam <= 'Z')) {
            key = (Key)((unsigned)Key::A + wParam - 'A');
        } else if ((wParam >= '0') && (wParam <= '9')) {
            key = (Key)((unsigned)Key::N0 + wParam - '0');
        } else {
            switch (wParam) {
            case VK_BACK: key = Key::BACKSPACE; break;
            case VK_SPACE: key = Key::SPACE; break;
            case VK_LEFT: key = Key::LEFT; break;
            case VK_RIGHT: key = Key::RIGHT; break;
            case VK_DOWN: key = Key::DOWN; break;
            case VK_UP: key = Key::UP; break;
            case VK_CONTROL: key = Key::CONTROL; break;
            case VK_SHIFT: key = Key::SHIFT; break;
            }
        }
        engine.Keyboard(phase, key);
    }

    void KeyboardChar(WPARAM wParam, LPARAM lParam, KeyboardPhase phase) {
        Key key = Key::None;
        if ((wParam >= 'a') && (wParam <= 'z')) {
            key = (Key)((unsigned)Key::a + wParam - 'a');
        } else if ((wParam >= 'A') && (wParam <= 'Z')) {
            key = (Key)((unsigned)Key::A + wParam - 'A');
        } else if ((wParam >= '0') && (wParam <= '9')) {
            key = (Key)((unsigned)Key::N0 + wParam - '0');
        } else {
            switch (wParam) {
            case ' ': key = Key::SPACE; break;
            case ',': key = Key::COMMA; break;
            case '.': key = Key::POINT; break;
            case '<': key = Key::LESS; break;
            case '>': key = Key::GREATER; break;
            case '/': key = Key::SLASH; break;
            case '\\': key = Key::BACKSLASH; break;
            case '?': key = Key::INTERROGATION; break;
            case '+': key = Key::PLUS; break;
            case '-': key = Key::MINUS; break;
            case '_': key = Key::UNDERSCORE; break;
            case '=': key = Key::EQUALS; break;
            }
        }
        engine.Keyboard(phase, key);
    }

public:
    Application(HINSTANCE hInstance, int nShowCmd, unsigned width, unsigned height)
    : engine(Parameters(Application::WindowProc, hInstance, "Sublime 2", this, nShowCmd, width, height)) {}

    void ProcessMessages() {
        MSG msg;
        BOOL ret = TRUE;
        while ((ret = GetMessage(&msg, NULL, 0, 0)) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    {
        Application application(hInstance, nShowCmd, 2436, 1125);
        application.ProcessMessages();
    }
    DEBUG_ONLY(Context::Check();)
    DEBUG_ONLY(_CrtDumpMemoryLeaks();)
    return 0;
}
