#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <cstdio>
#include <vector>
#include <cstdint>
#include <cmath>

#include "grid_fix.h"

#include <simcore/sim_state.hpp>
#include <simcore/sim_update.hpp>
#include <simcore/sim_hash.hpp>
#include <simcore/sim_initial_state.hpp>

static constexpr int WINDOW_W = 1000;
static constexpr int WINDOW_H = 800;

void draw_text(float x, float y, const char *s)
{
    glRasterPos2f(x, y);
    while (*s) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *s++);
    }
}

void draw_cube(float s)
{
    float h = s * 0.5f;
    glBegin(GL_QUADS);
    glVertex3f(-h,-h,-h); glVertex3f(h,-h,-h); glVertex3f(h,h,-h); glVertex3f(-h,h,-h);
    glVertex3f(-h,-h, h); glVertex3f(h,-h, h); glVertex3f(h,h, h); glVertex3f(-h,h, h);
    glVertex3f(-h,-h,-h); glVertex3f(-h,h,-h); glVertex3f(-h,h, h); glVertex3f(-h,-h, h);
    glVertex3f(h,-h,-h); glVertex3f(h,h,-h); glVertex3f(h,h, h); glVertex3f(h,-h, h);
    glVertex3f(-h,-h,-h); glVertex3f(h,-h,-h); glVertex3f(h,-h, h); glVertex3f(-h,-h, h);
    glVertex3f(-h,h,-h); glVertex3f(h,h,-h); glVertex3f(h,h, h); glVertex3f(-h,h, h);
    glEnd();
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO);
    glutInit(&argc, argv);

    SDL_Window *win = SDL_CreateWindow(
        "Sentinel — Deterministic Projectile (Side View)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SimState state = sim_initial_state();
    std::vector<SimState> history;
    history.reserve(20000);

    bool paused = false;
    uint64_t view_tick = 0;

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) paused = !paused;
                if (e.key.keysym.sym == SDLK_LEFT && view_tick > 0) view_tick--;
                if (e.key.keysym.sym == SDLK_RIGHT && view_tick + 1 < history.size()) view_tick++;
            }
        }

        if (!paused) {
            sim_update(state);
            history.push_back(state);
            view_tick = history.size() - 1;

            if ((state.tick % 60) == 0) {
                printf("tick=%llu hash=0x%016llx\n",
                       (unsigned long long)state.tick,
                       (unsigned long long)sim_hash(state));
            }
        }

        if (history.empty()) continue;
        const SimState &view = history[view_tick];

        glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* ---------- FIXED SIDE-ON CAMERA ---------- */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, (double)WINDOW_W / WINDOW_H, 0.1, 5000.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Camera fixed, looking across +X
        gluLookAt(
            -200.0, 120.0, 300.0,   // camera position
             300.0,   0.0,   0.0,   // look-at target
               0.0,   1.0,   0.0
        );

        // World grid anchored at origin
        draw_world_grid(0.0f, 0.0f);

        // Ghost trail
        for (size_t i = 0; i < history.size(); ++i) {
            float age = (float)(history.size() - i) / history.size();
            glColor4f(0.2f, 0.6f, 1.0f, 0.2f * age);
            glPushMatrix();
            glTranslatef(
                history[i].x.to_double(),
                history[i].y.to_double(),
                0.0f
            );
            draw_cube(2.0f);
            glPopMatrix();
        }

        // Current projectile
        glColor4f(1,1,1,1);
        glPushMatrix();
        glTranslatef(
            view.x.to_double(),
            view.y.to_double(),
            0.0f
        );
        draw_cube(4.0f);
        glPopMatrix();

        /* ---------- OVERLAY ---------- */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINDOW_W, 0, WINDOW_H, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        glColor3f(1,1,1);
        draw_text(10, WINDOW_H - 20, "SPACE: Pause/Resume");
        draw_text(10, WINDOW_H - 35, "LEFT/RIGHT: Scrub (paused)");

        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Tick: %llu   Mode: %s",
                 (unsigned long long)view.tick,
                 paused ? "PAUSED" : "RUNNING");
        draw_text(10, 20, buf);

        glEnable(GL_DEPTH_TEST);

        SDL_GL_SwapWindow(win);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
