#pragma once

#include <glad/glad.h>
#include "GameOfLife.hpp"
#include <glm/glm.hpp>

struct RenderSettings {
    glm::vec3 GradientLeft{ 1, 0, 1 };
    glm::vec3 GradientRight{ 1, 1, 0 };

    bool MarkSelectedCell{ false };
    glm::vec2 SelectedCell{ 0, 0 };

    float CellSize{ 5 };
};

class Renderer {
public:

    void Init();
    void Deinit();

    void Render(const BoardState&, float windowWidth, float windowHeight, const RenderSettings&);

    float CameraX{ 0 };
    float CameraY{ 0 };
    float CameraZoom{ 1 };

private:
};
