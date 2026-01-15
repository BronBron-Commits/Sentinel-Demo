#pragma once
#include <GL/gl.h>
#include <cmath>

static constexpr float GRID_SPACING = 20.0f;
static constexpr int GRID_EXTENT = 100;

// Draws a world-anchored grid that scrolls past the camera
inline void draw_world_grid(float cam_x, float cam_z)
{
    glColor4f(0.25f, 0.25f, 0.25f, 1.0f);
    glBegin(GL_LINES);

    float base_x = std::floor(cam_x / GRID_SPACING) * GRID_SPACING;
    float base_z = std::floor(cam_z / GRID_SPACING) * GRID_SPACING;

    for (int i = -GRID_EXTENT; i <= GRID_EXTENT; ++i) {
        // X-aligned lines
        glVertex3f(base_x + i * GRID_SPACING, 0,
                   base_z - GRID_EXTENT * GRID_SPACING);
        glVertex3f(base_x + i * GRID_SPACING, 0,
                   base_z + GRID_EXTENT * GRID_SPACING);

        // Z-aligned lines
        glVertex3f(base_x - GRID_EXTENT * GRID_SPACING, 0,
                   base_z + i * GRID_SPACING);
        glVertex3f(base_x + GRID_EXTENT * GRID_SPACING, 0,
                   base_z + i * GRID_SPACING);
    }

    glEnd();
}
