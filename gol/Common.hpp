#pragma once

#include <string_view>
#include <SDL.h>

void FatalError(std::string_view message, std::string_view caption, bool exit = true);
void FatalErrorSDL(std::string_view prefix, std::string_view caption, bool exit = true);
void YieldControl();

class Timer {
public:
    void Start() {
        m_Start = SDL_GetTicks();
    }

    Uint32 GetPassedMillis() const {
        return SDL_GetTicks() - m_Start;
    }

private:
    Uint32 m_Start{ 0 };
};

class RateCounter {
public:
    void Start() {
        m_Timer.Start();
    }

    void Update() {
        if (m_Timer.GetPassedMillis() >= 1000) {
            m_Timer.Start();
            m_Rate = m_Counted;
            m_Counted = 0;
        } else {
            m_Counted += 1;
        }
    }

    Uint32 GetRate() const {
        return m_Rate;
    }

private:
    Timer m_Timer{};
    Uint32 m_Rate{ 1 };
    Uint32 m_Counted{ 0 };
};
