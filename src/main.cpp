#include <SDL2/SDL.h>
#include "orchestra.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;

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

    bool running = true;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(ren, 8, 8, 16, 255);
        SDL_RenderClear(ren);

        // deterministic simulation will tick here

        SDL_RenderPresent(ren);
        SDL_Delay(16); // fixed timestep placeholder
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
