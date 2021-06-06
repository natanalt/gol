#include "Renderer.hpp"
#include "Common.hpp"
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Renderer::Init() {}
void Renderer::Deinit() {}

void Renderer::Render(const BoardState& state, float windowWidth, float windowHeight, const RenderSettings& settings) {
    // Current renderer is based on immediate mode OpenGL,
    // because I really can't be arsed to make a more "proper" solution with shaders,
    // buffers and all that.
    //
    // If I've enough time I may be able to pull off a non-compatibility-profile solution.
    //

    float cellScale = settings.CellSize;

    glm::mat4 projection = glm::ortho(0.0f, windowWidth, 0.0f, windowHeight, 0.0001f, 1000.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));

    glm::mat4 view(1.0f);
    view = glm::translate(view, -glm::vec3(CameraX, CameraY, 1));
    view = glm::scale(view, glm::vec3(CameraZoom, CameraZoom, 1));
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(view));

    float boardSizeWidth = state.GetWidth() * cellScale;
    float boardSizeHeight = state.GetHeight() * cellScale;

    glBegin(GL_QUADS);

    // Draw a black background
    glColor4f(0, 0, 0, 1);
    glVertex2f(0 * boardSizeWidth, 1 * boardSizeHeight);
    glColor4f(0, 0, 0, 1);
    glVertex2f(1 * boardSizeWidth, 1 * boardSizeHeight);
    glColor4f(0, 0, 0, 1);
    glVertex2f(1 * boardSizeWidth, 0 * boardSizeHeight);
    glColor4f(0, 0, 0, 1);
    glVertex2f(0 * boardSizeWidth, 0 * boardSizeHeight);

    glm::vec3 gradientStep = (settings.GradientRight - settings.GradientLeft) / boardSizeWidth;
    
    for (size_t y = 0; y < state.GetHeight(); y++) {
        for (size_t x = 0; x < state.GetWidth(); x++) {
            bool isSelected = settings.MarkSelectedCell && glm::vec2(x, y) == settings.SelectedCell;

            if (state.GetCellState(x, y) || isSelected) {
                float currentLeft = x * cellScale;

                float alpha = 1;
                glm::vec3 gradientLeft = settings.GradientLeft + gradientStep * currentLeft;
                glm::vec3 gradientRight = gradientLeft + gradientStep;
                
                if (isSelected) {
                    glColor4f(1, 1, 1, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 0), cellScale * (y + 1.0f * 1));
                    glColor4f(1, 1, 1, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 1), cellScale * (y + 1.0f * 1));
                    glColor4f(1, 1, 1, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 1), cellScale * (y + 1.0f * 0));
                    glColor4f(1, 1, 1, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 0), cellScale * (y + 1.0f * 0));
                } else {
                    glColor4f(gradientLeft.r, gradientLeft.g, gradientLeft.b, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 0), cellScale * (y + 1.0f * 1));
                    glColor4f(gradientRight.r, gradientRight.g, gradientRight.b, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 1), cellScale * (y + 1.0f * 1));
                    glColor4f(gradientRight.r, gradientRight.g, gradientRight.b, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 1), cellScale * (y + 1.0f * 0));
                    glColor4f(gradientLeft.r, gradientLeft.g, gradientLeft.b, alpha);
                    glVertex2f(cellScale * (x + 1.0f * 0), cellScale * (y + 1.0f * 0));
                }
            }
        }
    }
    
    glEnd();
}
