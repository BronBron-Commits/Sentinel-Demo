#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include <cstdio>
#include <cstdint>
#include <deque>

#include <simcore/sim_state.hpp>
#include <simcore/sim_update.hpp>
#include <simcore/sim_hash.hpp>
#include <simcore/sim_initial_state.hpp>

static constexpr int WINDOW_W = 1000;
static constexpr int WINDOW_H = 700;
static constexpr int HISTORY_LEN = 120;

/* Draw a cube centered at origin */
static void draw_cube(float s)
{
    float h = s * 0.5f;
    glBegin(GL_QUADS);

    // Front
    glVertex3f(-h,-h, h); glVertex3f( h,-h, h);
    glVertex3f( h, h, h); glVertex3f(-h, h, h);
    // Back
    glVertex3f(-h,-h,-h); glVertex3f(-h, h,-h);
    glVertex3f( h, h,-h); glVertex3f( h,-h,-h);
    // Left
    glVertex3f(-h,-h,-h); glVertex3f(-h,-h, h);
    glVertex3f(-h, h, h); glVertex3f(-h, h,-h);
    // Right
    glVertex3f( h,-h,-h); glVertex3f( h, h,-h);
    glVertex3f( h, h, h); glVertex3f( h,-h, h);
    // Top
    glVertex3f(-h, h,-h); glVertex3f(-h, h, h);
    glVertex3f( h, h, h); glVertex3f( h, h,-h);
    // Bottom
    glVertex3f(-h,-h,-h); glVertex3f( h,-h,-h);
    glVertex3f( h,-h, h); glVertex3f(-h,-h, h);

    glEnd();
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *win = SDL_CreateWindow(
        "Sentinel 3D Multiverse",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    SDL_GLContext glctx = SDL_GL_CreateContext(win);
    SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.06f, 0.1f, 1.0f);

    // ---- sim-core ----
    SimState state = sim_initial_state();
    state.vx = Fixed::from_int(2);

    std::deque<SimState> history;

    const uint32_t FIXED_DT_MS = 16;
    uint32_t last_tick = SDL_GetTicks();

    bool running = true;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                running = false;

        uint32_t now = SDL_GetTicks();
        while (now - last_tick >= FIXED_DT_MS)
        {
            history.push_front(state);
            if ((int)history.size() > HISTORY_LEN)
                history.pop_back();

            sim_update(state);

            uint64_t h = sim_hash(state);
            printf("tick=%llu hash=0x%016llx\n",
                   (unsigned long long)state.tick,
                   (unsigned long long)h);

            last_tick += FIXED_DT_MS;
        }

        // ---- render ----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)WINDOW_W / (float)WINDOW_H;
        gluPerspective(60.0, aspect, 0.1, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, -1.0f, -14.0f);

        // ---- ghosts ----
        for (size_t i = 0; i < history.size(); ++i)
        {
            float t = (float)i / history.size();
            float z = -t * 10.0f;

            glColor4f(0.4f, 0.6f, 1.0f, 0.25f);
            glPushMatrix();
            glTranslatef(history[i].x.to_double() * 0.05f, 0.0f, z);
            draw_cube(0.4f);
            glPopMatrix();
        }

        // ---- current timeline ----
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glPushMatrix();
        glTranslatef(state.x.to_double() * 0.05f, 0.0f, 0.0f);
        draw_cube(0.7f);
        glPopMatrix();

        SDL_GL_SwapWindow(win);
    }

    SDL_GL_DeleteContext(glctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
