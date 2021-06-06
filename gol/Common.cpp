#include "Common.hpp"
#include <iostream>
#include <sstream>
#include <SDL.h>

#ifndef _WIN32
#error Currently only Windows platforms are supported
#endif

#include <windows.h>

void FatalError(std::string_view message, std::string_view caption, bool exit) {
    MessageBoxA(nullptr, message.data(), caption.data(), MB_OK | MB_ICONERROR);
    if (exit) {
        std::exit(1);
        while (true);
    }
}

void FatalErrorSDL(std::string_view prefix, std::string_view caption, bool exit) {
    std::ostringstream stream;
    stream << prefix;
    stream << SDL_GetError();
    FatalError(stream.str(), caption, exit);
}

void YieldControl() {
    SwitchToThread();
}
