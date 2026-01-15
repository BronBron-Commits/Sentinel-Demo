#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

// ---- deterministic sim state ----
typedef struct {
    int32_t x;   // fixed-point (16.16)
    int32_t vx;  // fixed-point velocity
} SimState;

static void sim_step(SimState *s)
{
    s->x += s->vx; // pure deterministic update
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow(
        "Sentinel Deterministic Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1, SDL_RENDERER_ACCELERATED
    );

    // ---- init sim ----
    SimState sim = {
        .x  = 100 << 16,
        .vx = 1 << 16  // exactly 1 unit per tick
    };

    const int FIXED_DT_MS = 16;
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
            sim_step(&sim);
            last_tick += FIXED_DT_MS;
        }

        // ---- render ----
        SDL_SetRenderDrawColor(ren, 8, 8, 16, 255);
        SDL_RenderClear(ren);

        int screen_x = sim.x >> 16;

        SDL_Rect dot = {
            .x = screen_x,
            .y = 300,
            .w = 8,
            .h = 8
        };

        SDL_SetRenderDrawColor(ren, 220, 220, 255, 255);
        SDL_RenderFillRect(ren, &dot);

        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
