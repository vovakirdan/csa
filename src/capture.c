#include "capture.h"

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

int capture_screen(unsigned char** buffer, int* width, int* height) {
#ifdef _WIN32
    // Захват экрана через GDI
#elif __linux__
    // Захват экрана через X11
#endif
    return 0;
}
