#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

#include <simcore/sim_state.hpp>
#include <simcore/sim_update.hpp>
#include <simcore/sim_hash.hpp>
#include <simcore/sim_initial_state.hpp>

static constexpr int WINDOW_W = 1000;
static constexpr int WINDOW_H = 700;
static constexpr uint32_t FIXED_DT_MS = 16;
static constexpr float X_SCALE = 0.03f;
static constexpr float Z_SPREAD = 25.0f;

/* ---------------- helpers ---------------- */

static void draw_cube(float s)
{
    float h = s * 0.5f;
    glBegin(GL_QUADS);
    glVertex3f(-h,-h, h); glVertex3f( h,-h, h); glVertex3f( h, h, h); glVertex3f(-h, h, h);
    glVertex3f(-h,-h,-h); glVertex3f(-h, h,-h); glVertex3f( h, h,-h); glVertex3f( h,-h,-h);
    glVertex3f(-h,-h,-h); glVertex3f(-h,-h, h); glVertex3f(-h, h, h); glVertex3f(-h, h,-h);
    glVertex3f( h,-h,-h); glVertex3f( h, h,-h); glVertex3f( h, h, h); glVertex3f( h,-h, h);
    glVertex3f(-h, h,-h); glVertex3f(-h, h, h); glVertex3f( h, h, h); glVertex3f( h, h,-h);
    glVertex3f(-h,-h,-h); glVertex3f( h,-h,-h); glVertex3f( h,-h, h); glVertex3f(-h,-h, h);
    glEnd();
}

static void draw_grid()
{
    glColor4f(0.2f, 0.25f, 0.35f, 0.4f);
    glBegin(GL_LINES);
    for (int i = -20; i <= 20; ++i)
    {
        glVertex3f(i, 0, -40); glVertex3f(i, 0, 10);
        glVertex3f(-20, 0, i); glVertex3f(20, 0, i);
    }
    glEnd();
}

static void draw_vector(float x, float y, float dx, float dy, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex3f(x, y, 0);
    glVertex3f(x + dx, y + dy, 0);
    glEnd();
}

/* ---------------- main ---------------- */

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
        "Sentinel Rewind Inspector",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    SDL_GLContext glctx = SDL_GL_CreateContext(win);
    SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.06f, 0.1f, 1.0f);

    /* ---- authoritative sim ---- */
    SimState sim_state = sim_initial_state();
    sim_state.vx = Fixed::from_double(0.6);
    sim_state.vy = Fixed::from_double(1.2);

    std::vector<SimState> snapshots;
    std::vector<uint64_t> hashes;

    snapshots.push_back(sim_state);
    hashes.push_back(sim_hash(sim_state));

    uint64_t sim_tick = 0;
    uint64_t view_tick = 0;
    bool paused = false;
    bool deterministic_ok = true;

    uint32_t last_time = SDL_GetTicks();
    SDL_Event e;

    while (true)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                goto exit;

            if (e.type == SDL_KEYDOWN)
            {
                bool shift = (e.key.keysym.mod & KMOD_SHIFT);
                switch (e.key.keysym.sym)
                {
                    case SDLK_SPACE: paused = !paused; break;
                    case SDLK_r: view_tick = 0; break;
                    case SDLK_LEFT:
                        view_tick = (view_tick > (shift ? 10 : 1)) ? view_tick - (shift ? 10 : 1) : 0;
                        break;
                    case SDLK_RIGHT:
                        view_tick += (shift ? 10 : 1);
                        break;
                }
            }
        }

        uint32_t now = SDL_GetTicks();
        if (!paused)
        {
            while (now - last_time >= FIXED_DT_MS)
            {
                sim_update(sim_state);
                sim_tick++;

                snapshots.push_back(sim_state);
                uint64_t h = sim_hash(sim_state);
                hashes.push_back(h);

                printf("tick=%llu hash=0x%016llx\n",
                       (unsigned long long)sim_tick,
                       (unsigned long long)h);

                last_time += FIXED_DT_MS;
            }
        }

        if (view_tick >= snapshots.size())
            view_tick = snapshots.size() - 1;

        SimState view = snapshots[view_tick];

        deterministic_ok = (sim_hash(view) == hashes[view_tick]);

        /* ---- render ---- */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(65.0, (float)WINDOW_W / WINDOW_H, 0.1, 300.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, -1.6f, -26.0f);
        glRotatef(12, 1, 0, 0);

        draw_grid();

        /* ---- ghosts ---- */
        for (uint64_t i = 0; i < view_tick; ++i)
        {
            float t = (float)i / view_tick;
            float z = -t * Z_SPREAD;
            double dx = snapshots[i].x.to_double() - view.x.to_double();

            glColor4f(0.4f, 0.6f, 1.0f, 0.2f);
            glPushMatrix();
            glTranslatef(dx * X_SCALE, snapshots[i].y.to_double(), z);
            draw_cube(0.35f);
            glPopMatrix();
        }

        /* ---- current ---- */
        glColor3f(1,1,1);
        glPushMatrix();
        glTranslatef(0, view.y.to_double(), 0);
        draw_cube(0.8f);
        glPopMatrix();

        /* ---- vectors ---- */
        draw_vector(0, view.y.to_double(),
                    view.vx.to_double(), view.vy.to_double(),
                    0,1,0);   // velocity

        draw_vector(0, view.y.to_double(),
                    0, -0.5f,
                    1,0,0);   // gravity

        /* ---- overlay ---- */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINDOW_W, WINDOW_H, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3f(deterministic_ok ? 0.2f : 1.0f,
                  deterministic_ok ? 1.0f : 0.0f,
                  0.2f);

        glBegin(GL_QUADS);
        glVertex2f(0,0); glVertex2f(WINDOW_W,0);
        glVertex2f(WINDOW_W,24); glVertex2f(0,24);
        glEnd();

        SDL_GL_SwapWindow(win);
    }

exit:
    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
