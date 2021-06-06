#include "App.hpp"
#include "Common.hpp"

#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

class FramerateController {
public:
    void Start() {
        m_FPSCounter.Start();
    }

    void BeginFrame() {
        m_FrameTimeCounter.Start();
    }

    void EndFrame() {
        auto frameMillis = m_FrameTimeCounter.GetPassedMillis();

        m_Delta = 1.0f / float(frameMillis);
        m_FPSCounter.Update();

        int sleep = GetMaxSleepMillis() - frameMillis;
        if (sleep > 0) {
            SDL_Delay(Uint32(sleep));
        }
    }

    Uint32 GetFramerateCap() const { return m_FramerateCap; }
    void SetFramerateCap(Uint32 f) { m_FramerateCap = f; }

    Uint32 GetFPS() const { return m_FPSCounter.GetRate(); }
    float GetDelta() const { return m_Delta; }

    Uint32 GetMaxSleepMillis() const {
        return Uint32(1000.0f / m_FramerateCap);
    }

private:
    Uint32 m_FramerateCap{ 60 };

    Timer m_FrameTimeCounter{};
    RateCounter m_FPSCounter{};

    float m_Delta{ 1 };
};

void App::Run() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        FatalErrorSDL("SDL failed to initialise:\n", "SDL Initialisation Error");
    SDL_SetMainReady();

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_Window = SDL_CreateWindow(
        "nat's game of life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_Window) FatalErrorSDL("SDL failed to create a window:\n", "SDL Initialisation Error");

    m_Context = SDL_GL_CreateContext(m_Window);
    if (!m_Context) FatalErrorSDL("SDL failed to create an OpenGL context:\n", "SDL Initialisation Error");
    gladLoadGL();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_Context);
    ImGui_ImplOpenGL3_Init("#version 130");

    m_Renderer.Init();

    FramerateController framerateController;
    RenderSettings renderSettings;

    auto localToWorld = [&](glm::vec2 local) -> glm::vec2 {
        return (local + glm::vec2(m_Renderer.CameraX, m_Renderer.CameraY)) / m_Renderer.CameraZoom;
    };

    auto worldToLocal = [&](glm::vec2 world) -> glm::vec2 {
        return (world * m_Renderer.CameraZoom) - glm::vec2(m_Renderer.CameraX, m_Renderer.CameraY);
    };

    auto isOnBoard = [&](glm::vec2 world) -> bool {
        if (world.x < 0 || world.y < 0)
            return false;
        if (world.x >= (m_IterationController.GetBoardWidth() * renderSettings.CellSize))
            return false;
        if (world.y >= (m_IterationController.GetBoardHeight() * renderSettings.CellSize))
            return false;
        return true;
    };

    auto swapY = [&](auto y) {
        return m_WindowHeight - y;
    };

    bool isMovingCamera = false;
    glm::vec2 localMoveHoldPoint;
    glm::vec2 worldMoveHoldPoint;
    glm::vec2 lastSelectedCell(0);

    framerateController.Start();
    m_IterationController.Pause();

    m_IsRunning = true;
    while (m_IsRunning) {
        framerateController.BeginFrame();
        auto delta = framerateController.GetDelta();

        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            ImGui_ImplSDL2_ProcessEvent(&evt);

            switch (evt.type) {
                case SDL_QUIT:
                    m_IsRunning = false;
                    break;
            }

            if (!io.WantCaptureMouse) {
                switch (evt.type) {
                    case SDL_MOUSEWHEEL:
                    {
                        // TODO: fix zoom
                        auto local = glm::vec2(evt.button.x, swapY(evt.button.y));
                        auto world = glm::vec2(m_Renderer.CameraX, m_Renderer.CameraY);
                        auto zoom1 = m_Renderer.CameraZoom;
                        m_Renderer.CameraZoom += 0.1f * delta * evt.wheel.y;
                        auto zoom2 = m_Renderer.CameraZoom;
                        auto newWorld = (((local + world) / zoom1) * zoom2) - local / zoom1 * zoom2;
                        m_Renderer.CameraX = newWorld.x;
                        m_Renderer.CameraY = newWorld.y;
                    }
                    break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        if (evt.button.button == SDL_BUTTON_MIDDLE) {
                            if (evt.button.state == SDL_PRESSED) {
                                isMovingCamera = true;
                                localMoveHoldPoint = glm::vec2(evt.button.x, swapY(evt.button.y));
                                worldMoveHoldPoint = glm::vec2(m_Renderer.CameraX, m_Renderer.CameraY);
                            } else {
                                isMovingCamera = false;
                            }
                        }
                        break;
                    
                    case SDL_MOUSEMOTION:
                        if ((evt.motion.state & SDL_BUTTON_MMASK) && isMovingCamera) {
                            auto movementDelta = glm::vec2(evt.motion.x, swapY(evt.motion.y)) - localMoveHoldPoint;
                            auto newPosition = worldMoveHoldPoint - movementDelta;
                            m_Renderer.CameraX = newPosition.x;
                            m_Renderer.CameraY = newPosition.y;
                        }
                        auto mouseWorld = localToWorld(glm::vec2(evt.motion.x, swapY(evt.motion.y)));
                        renderSettings.MarkSelectedCell = isOnBoard(mouseWorld);
                        renderSettings.SelectedCell = glm::floor(mouseWorld / renderSettings.CellSize);


                        
                        break;
                }
            }
        }

        // Clear buffers
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update the viewport
        SDL_GetWindowSize(m_Window, &m_WindowWidth, &m_WindowHeight);
        glViewport(0, 0, m_WindowWidth, m_WindowHeight);

        // Get mouse position
        int mouseX, mouseY;
        auto mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        mouseY = swapY(mouseY);
        auto mouseLocal = glm::vec2(mouseX, mouseY);
        auto mouseWorld = localToWorld(mouseLocal);

        // Mouse drawing
        if (m_IterationController.IsPaused() && (mouseState & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))) {
            bool target = mouseState & SDL_BUTTON_LMASK;
            m_IterationController.GetMutRenderBoard().SetCellLine(
                lastSelectedCell.x, lastSelectedCell.y,
                renderSettings.SelectedCell.x, renderSettings.SelectedCell.y,
                target
            );

            //std::printf("(%d, %d) has %d neigbhors\n", (int)renderSettings.SelectedCell.x, (int)renderSettings.SelectedCell.y, m_IterationController.GetMutRenderBoard().CountNeighbors(renderSettings.SelectedCell.x, renderSettings.SelectedCell.y));
        }

        // Render the board
        if (m_Renderer.CameraZoom < 0.001f)
            m_Renderer.CameraZoom = 0.001f;

        m_IterationController.Process(delta);
        m_Renderer.Render(m_IterationController.GetRenderBoard(), float(m_WindowWidth), float(m_WindowHeight), renderSettings);

        // Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(m_Window);
        ImGui::NewFrame();

        ImGui::Begin("nat's game of life");

        ImGui::Text("%d FPS", framerateController.GetFPS());
        ImGui::Spacing();

        ImGui::Text("Mouse local position: (%d, %d)", mouseX, mouseY);
        ImGui::Text("Mouse world position: (%f, %f)", mouseWorld.x, mouseWorld.y);

        if (ImGui::Button("Clear")) {
            m_IterationController.GetMutRenderBoard().Clear();
        }

        if (ImGui::CollapsingHeader("Camera control")) {
            ImGui::Text("Camera position: (%f, %f)", m_Renderer.CameraX, m_Renderer.CameraY);
            ImGui::Text("Camera zoom: %.4fx", m_Renderer.CameraZoom);

            if (ImGui::Button("+") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraZoom += 0.1f * delta;
            ImGui::SameLine();
            if (ImGui::Button("-") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraZoom -= 0.1f * delta;
            ImGui::SameLine(60);
            if (ImGui::Button("<") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraX -= 25.0f * delta;
            ImGui::SameLine();
            if (ImGui::Button(">") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraX += 25.0f * delta;
            ImGui::SameLine();
            if (ImGui::Button("^") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraY += 25.0f * delta;
            ImGui::SameLine();
            if (ImGui::Button("v") || (ImGui::IsItemActive() && ImGui::IsItemHovered())) m_Renderer.CameraY -= 25.0f * delta;

            if (ImGui::Button("Reset camera position")) {
                m_Renderer.CameraX = 0;
                m_Renderer.CameraY = 0;
            }

            if (ImGui::Button("Reset camera zoom"))
                m_Renderer.CameraZoom = 1;
        }
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Gradient options")) {
            ImGui::ColorEdit3("Left", glm::value_ptr(renderSettings.GradientLeft));
            ImGui::ColorEdit3("Right", glm::value_ptr(renderSettings.GradientRight));
        }
        ImGui::Spacing();
        ImGui::Spacing();

        m_IterationController.RenderImgui();

        ImGui::End();

        // Render ImGui onto the main window
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(m_Window);

        framerateController.EndFrame();
        lastSelectedCell = renderSettings.SelectedCell;
    }

    m_Renderer.Deinit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}

int main() {
    App().Run();
    return 0;
}
