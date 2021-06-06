#include "GameOfLife.hpp"
#include "Common.hpp"
#include <imgui.h>
#include <cinttypes>
#include <cmath>

static float t = 0;

void IterationController::Process(float delta) {
    if (m_IsPaused)
        return;
    
    float nya = 1.0f / float(m_IterationsPerSecond);
    t += delta;
    if (t < nya) {
        return;
    }
    t = 0;

    auto iterations = std::ceil(delta * m_IterationsPerSecond);
    for (int i = 0; i < iterations; i++) {
        DoIteration();
        m_IterationCounter += 1;
    }
}

void IterationController::DoIteration() {
    BoardState original = m_RenderBoard;
    //m_RenderBoard.Clear();

    for (int y = 0; y < m_RenderBoard.GetHeight(); y++) {
        for (int x = 0; x < m_RenderBoard.GetWidth(); x++) {
            int neighbors = original.CountNeighbors(x, y);
            if (original.GetCellState(x, y)) {
                if (neighbors < 2) {
                    m_RenderBoard.SetCellState(x, y, false);
                }
                if (neighbors > 3) {
                    m_RenderBoard.SetCellState(x, y, false);
                }
            } else {
                if (neighbors == 3) {
                    m_RenderBoard.SetCellState(x, y, true);
                }
            }
        }
    }
}

void IterationController::RenderImgui() {
    if (ImGui::CollapsingHeader("Iteration options")) {
        if (m_IsPaused)
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Iteration Paused");
        else
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Iteration Running");

        if (ImGui::Button("|| Pause")) Pause();
        ImGui::SameLine();
        if (ImGui::Button("> Resume")) Resume();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("%" PRId64 " iterations total", m_IterationCounter);
        if (m_IsPaused) {
            if (ImGui::Button("Reset iteration count")) {
                ResetIterationCounter();
            }
            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::InputInt("Iterations per second", &m_IterationsPerSecond);

        } else {
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("More options are available when the iteration is paused");
        }
    }
}
