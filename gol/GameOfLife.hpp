#pragma once

#include "Common.hpp"

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <glm/glm.hpp>
#include <atomic>

#include <iostream>

typedef int CellType;

class BoardState {
public:
    BoardState(size_t w, size_t h)
        : m_States(w * h)
        , m_Width(w)
        , m_Height(h) {}

    static BoardState GenerateTestBoard() {
        BoardState result(100, 100);

        for (size_t x = 0; x < result.GetWidth(); x++) {
            result.SetCellState(x, 0, true);
            result.SetCellState(x, result.GetHeight() - 1, true);
        }

        for (size_t y = 1; y < result.GetHeight() - 1; y++) {
            result.SetCellState(0, y, true);
            result.SetCellState(result.GetWidth() - 1, y, true);
        }

        return result;
    }

    bool IsInBounds(size_t x, size_t y) const {
        return x < m_Width && y < m_Height;
    }

    bool GetCellState(int x, int y) const {
        x = ClampX(x);
        y = ClampY(y);
        return m_States[y * m_Width + x] != 0;
    }

    void SetCellState(int x, int y, bool state) {
        x = ClampX(x);
        y = ClampY(y);
        m_States[y * m_Width + x] = state ? 1 : 0;
    }

    size_t GetWidth() const {
        return m_Width;
    }

    size_t GetHeight() const {
        return m_Height;
    }

    size_t ClampX(int x) const {
        if (x < 0) return m_Width + x;
        if (x >= m_Width) return x - m_Width;
        return x;
    }

    size_t ClampY(int y) const {
        if (y < 0) return m_Height + y;
        if (y >= m_Height) return y - m_Height;
        return y;
    }

    void SetCellLine(int x1, int y1, int x2, int y2, bool state) {
        x1 = ClampX(x1);
        x2 = ClampX(x2);
        y1 = ClampY(y1);
        y2 = ClampY(y2);

        glm::vec2 start(x1, y1);
        glm::vec2 end(x2, y2);
        glm::vec2 direction = glm::normalize(end - start);
        float length = glm::length(end - start);
        float travelled = 0;
        
        glm::vec2 current = start;
        while (travelled <= length) {
            SetCellState(current.x, current.y, state);
            current += direction;
            travelled += 1;
        }
    }

    void Clear() {
        for (int i = 0; i < m_States.size(); i++)
            m_States[i] = 0;
    }

    int CountNeighbors(int x, int y) {
        int neighbors = 0;
        neighbors += GetCellState(x - 1, y - 1) ? 1 : 0;
        neighbors += GetCellState(x + 0, y - 1) ? 1 : 0;
        neighbors += GetCellState(x + 1, y - 1) ? 1 : 0;
        neighbors += GetCellState(x - 1, y + 0) ? 1 : 0;
        neighbors += GetCellState(x + 1, y + 0) ? 1 : 0;
        neighbors += GetCellState(x - 1, y + 1) ? 1 : 0;
        neighbors += GetCellState(x + 0, y + 1) ? 1 : 0;
        neighbors += GetCellState(x + 1, y + 1) ? 1 : 0;
        return neighbors;
    }

private:
    size_t m_Width;
    size_t m_Height;
    std::vector<CellType> m_States;
};

class IterationController {
public:
    IterationController(size_t boardWidth, size_t boardHeight)
        : m_RenderBoard(boardWidth, boardHeight){}

    void Pause() { m_IsPaused = true; }
    void Resume() { m_IsPaused = false; }

    void Process(float delta);
    void DoIteration();

    const BoardState& GetRenderBoard() { return m_RenderBoard; };
    BoardState& GetMutRenderBoard() { return m_RenderBoard; };

    int GetBoardWidth() const { return m_RenderBoard.GetWidth(); }
    int GetBoardHeight() const { return m_RenderBoard.GetHeight(); }
    bool IsPaused() const { return m_IsPaused; }

    long long GetIterationCounter() const { return m_IterationCounter; }
    void ResetIterationCounter() {
        m_IterationCounter = 0;
    }

    void RenderImgui();

private:
    BoardState m_RenderBoard;
    bool m_IsPaused{ false };
    long long m_IterationCounter{ 0 };
    int m_IterationsPerSecond{ 10 };
};
