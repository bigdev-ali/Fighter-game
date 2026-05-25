#include <SDL3/SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL failed to init: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Fighter Game", 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    float p1x = 100, p1y = 400;
    float p2x = 650, p2y = 400;

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        // Get keyboard state
        const bool *keys = SDL_GetKeyboardState(NULL);

        // Player 1 movement (WASD)
        if (keys[SDL_SCANCODE_A])
            p1x -= 5;
        if (keys[SDL_SCANCODE_D])
            p1x += 5;

        // Player 2 movement (Arrow keys)
        if (keys[SDL_SCANCODE_LEFT])
            p2x -= 5;
        if (keys[SDL_SCANCODE_RIGHT])
            p2x += 5;

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw player 1 (red)
        SDL_FRect player1 = {p1x, p1y, 50, 100};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &player1);

        // Draw player 2 (blue)
        SDL_FRect player2 = {p2x, p2y, 50, 100};
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &player2);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}