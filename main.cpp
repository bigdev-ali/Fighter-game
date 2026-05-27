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

    // Player positions
    float p1x = 100, p1y = 400;
    float p2x = 650, p2y = 400;

    // Velocities
    float p1vy = 0, p2vy = 0;

    // Jump states
    bool p1jumping = false, p2jumping = false;

    // Physics
    float gravity = 1500.0f;
    float jumpForce = -600.0f;
    float groundY = 400.0f;

    Uint64 lastTime = SDL_GetTicks();

    bool running = true;
    SDL_Event event;

    while (running)
    {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            // Jump on key press (not hold)
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.scancode == SDL_SCANCODE_W && !p1jumping)
                {
                    p1vy = jumpForce;
                    p1jumping = true;
                }
                if (event.key.scancode == SDL_SCANCODE_UP && !p2jumping)
                {
                    p2vy = jumpForce;
                    p2jumping = true;
                }
            }
        }

        // Get keyboard state
        const bool *keys = SDL_GetKeyboardState(NULL);

        // Player 1 movement (WASD)
        if (keys[SDL_SCANCODE_A]) p1x -= 300 * deltaTime;
        if (keys[SDL_SCANCODE_D]) p1x += 300 * deltaTime;

        // Player 2 movement (Arrow keys)
        if (keys[SDL_SCANCODE_LEFT]) p2x -= 300 * deltaTime;
        if (keys[SDL_SCANCODE_RIGHT]) p2x += 300 * deltaTime;

        // Apply gravity to player 1
        p1vy += gravity * deltaTime;
        p1y += p1vy * deltaTime;

        // Player 1 land on ground
        if (p1y >= groundY)
        {
            p1y = groundY;
            p1vy = 0;
            p1jumping = false;
        }

        // Apply gravity to player 2
        p2vy += gravity * deltaTime;
        p2y += p2vy * deltaTime;

        // Player 2 land on ground
        if (p2y >= groundY)
        {
            p2y = groundY;
            p2vy = 0;
            p2jumping = false;
        }

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