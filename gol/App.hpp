#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "GameOfLife.hpp"
#include "Renderer.hpp"

class Renderer;

class App {
public:
    App()
        : m_IterationController(100, 100) {}

    void Run();
    bool IsRunning() const { return m_IsRunning; }

    IterationController& GetIterationController() { return m_IterationController; }
    Renderer& GetRenderer() { return m_Renderer; }
    
    SDL_Window* GetWindow() { return m_Window; }
    int GetWindowWidth() const { return m_WindowWidth; }
    int GetWindowHeight() const { return m_WindowHeight; }

private:
    bool m_IsRunning{ false };
    
    IterationController m_IterationController;
    Renderer m_Renderer{};

    SDL_Window* m_Window{ nullptr };
    SDL_GLContext m_Context{};
    int m_WindowWidth{};
    int m_WindowHeight{};
};
