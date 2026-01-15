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
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
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
    bool rewound = false;
    uint64_t view_tick = 0;

    bool markers_computed = false;
    uint64_t apex_tick = 0;
    uint64_t impact_tick = 0;

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    if (paused && rewound) {
                        state = history[view_tick];
                        history.resize(view_tick + 1);
                        rewound = false;
                    }
                    paused = !paused;
                }

                if (e.key.keysym.sym == SDLK_LEFT) {
                    paused = true;
                    rewound = true;
                    if (view_tick > 0)
                        view_tick--;
                }

                if (e.key.keysym.sym == SDLK_RIGHT) {
                    paused = true;
                    rewound = true;
                    if (view_tick + 1 < history.size())
                        view_tick++;
                }
            }
        }

        if (!paused) {
            sim_update(state);
            history.push_back(state);
            view_tick = history.size() - 1;

            if (!markers_computed && history.size() > 2) {
                double max_y = history[0].y.to_double();
                apex_tick = 0;

                for (size_t i = 1; i < history.size(); ++i) {
                    double y = history[i].y.to_double();
                    if (y > max_y) {
                        max_y = y;
                        apex_tick = i;
                    }
                    if (y <= 0.0) {
                        impact_tick = i;
                        markers_computed = true;
                        break;
                    }
                }
            }

            if ((state.tick % 60) == 0) {
                printf("tick=%llu hash=0x%016llx\n",
                       (unsigned long long)state.tick,
                       (unsigned long long)sim_hash(state));
            }
        }

        if (history.empty())
            continue;

        const SimState &view = history[view_tick];
        bool blink = markers_computed &&
                     (view_tick == apex_tick || view_tick == impact_tick);

        glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* ---------- FIXED SIDE-ON CAMERA ---------- */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, (double)WINDOW_W / WINDOW_H, 0.1, 5000.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(
            -200.0, 120.0, 300.0,
             300.0,   0.0,   0.0,
               0.0,   1.0,   0.0
        );

        draw_world_grid(0.0f, 0.0f);

        for (size_t i = 0; i < history.size(); ++i) {
            float age = (float)(history.size() - i) / history.size();
            if (blink && i == view_tick)
                glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
            else
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

        /* ---------- CURRENT PROJECTILE + MARKERS ---------- */
glPushMatrix();
glTranslatef(
    view.x.to_double(),
    view.y.to_double(),
    0.0f
);

/* cube */
glColor4f(1,1,1,1);
draw_cube(4.0f);

/* apex / impact ring */
if (blink) {
    glBegin(GL_LINE_LOOP);
    for (int j = 0; j < 32; ++j) {
        float a = (float)j / 32.0f * 2.0f * 3.1415926f;
        glVertex3f(cosf(a) * 6.0f, sinf(a) * 6.0f, 0.0f);
    }
    glEnd();
}

/* velocity vector arrow */
{
    const double vx = view.vx.to_double();
    const double vy = view.vy.to_double();

    const double speed = std::sqrt(vx * vx + vy * vy);
    if (speed > 1e-6) {
        const double scale = 8.0;  // visual scaling factor
        const double dx = vx * scale;
        const double dy = vy * scale;

        glColor4f(0.0f, 1.0f, 0.0f, 1.0f); // green

        /* shaft */
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(dx, dy, 0.0f);
        glEnd();

        /* arrow head */
        const double nx = dx / speed;
        const double ny = dy / speed;
        const double ah = 2.0; // arrow head size

        glBegin(GL_TRIANGLES);
        glVertex3f(dx, dy, 0.0f);
        glVertex3f(
            dx - nx * ah - ny * ah * 0.5,
            dy - ny * ah + nx * ah * 0.5,
            0.0f
        );
        glVertex3f(
            dx - nx * ah + ny * ah * 0.5,
            dy - ny * ah - nx * ah * 0.5,
            0.0f
        );
        glEnd();
    }
}

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

        char hashbuf[128];
        snprintf(hashbuf, sizeof(hashbuf),
                 "Hash: 0x%016llx",
                 (unsigned long long)sim_hash(view));
        draw_text(10, 35, hashbuf);

        glEnable(GL_DEPTH_TEST);
        SDL_GL_SwapWindow(win);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
