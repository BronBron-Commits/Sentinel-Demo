#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdint>
#include <deque>
#include <cmath>

#include <simcore/sim_state.hpp>
#include <simcore/sim_update.hpp>
#include <simcore/sim_hash.hpp>
#include <simcore/sim_initial_state.hpp>

static constexpr int WINDOW_W = 900;
static constexpr int WINDOW_H = 600;
static constexpr int HISTORY_LEN = 200;

// Convert sim X to screen
static int sim_x_to_screen(const SimState& s)
{
    double x = s.x.to_double();
    int px = static_cast<int>(x) % (WINDOW_W - 40);
    if (px < 0) px += (WINDOW_W - 40);
    return px + 20;
}

// Draw glowing square (multi-pass)
static void draw_glow(SDL_Renderer* ren, int x, int y, int size,
                      SDL_Color color, int layers)
{
    for (int i = layers; i >= 1; --i)
    {
        uint8_t a = static_cast<uint8_t>(color.a / (i + 1));
        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, a);

        SDL_Rect r = {
            x - i,
            y - i,
            size + i * 2,
            size + i * 2
        };
        SDL_RenderFillRect(ren, &r);
    }

    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    SDL_Rect core = { x, y, size, size };
    SDL_RenderFillRect(ren, &core);
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
        "Sentinel Multiverse — Deterministic Timeline",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED
    );

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    // ---- sim-core state ----
    SimState state = sim_initial_state();
    state.vx = Fixed::from_int(3);   // deterministic motion
    state.vy = Fixed::from_int(0);

    std::deque<SimState> history;

    const uint32_t FIXED_DT_MS = 16;
    uint32_t last_tick = SDL_GetTicks();

    bool running = true;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
        }

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

        // ---- background ----
        SDL_SetRenderDrawColor(ren, 5, 6, 12, 255);
        SDL_RenderClear(ren);

        // ---- ghost trails ----
        for (size_t i = 0; i < history.size(); ++i)
        {
            float t = (float)i / (float)history.size();
            uint8_t alpha = static_cast<uint8_t>(200 * (1.0f - t) * (1.0f - t));

            int x = sim_x_to_screen(history[i]);
            int y = WINDOW_H / 2;

            SDL_Color ghost_color = {
                80,
                static_cast<uint8_t>(120 + 100 * (1.0f - t)),
                255,
                alpha
            };

            draw_glow(ren, x, y, 6, ghost_color, 2);
        }

        // ---- current timeline (bright + strong glow) ----
        int cx = sim_x_to_screen(state);
        int cy = WINDOW_H / 2;

        SDL_Color current_color = { 255, 255, 255, 255 };
        draw_glow(ren, cx, cy, 12, current_color, 5);

        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
