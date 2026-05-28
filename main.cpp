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

    // Health
    float p1health = 100, p2health = 100;

    // Attack states
    bool p1attacking = false, p2attacking = false;
    int p1attackTimer = 0, p2attackTimer = 0;

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

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                // Jump
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

                // Attack
                if (event.key.scancode == SDL_SCANCODE_F && !p1attacking)
                {
                    p1attacking = true;
                    p1attackTimer = 15;
                }
                if (event.key.scancode == SDL_SCANCODE_K && !p2attacking)
                {
                    p2attacking = true;
                    p2attackTimer = 15;
                }
            }
        }

        // Get keyboard state
        const bool *keys = SDL_GetKeyboardState(NULL);

        // Player 1 movement (WASD)
        if (keys[SDL_SCANCODE_A])
            p1x -= 300 * deltaTime;
        if (keys[SDL_SCANCODE_D])
            p1x += 300 * deltaTime;

        // Player 2 movement (Arrow keys)
        if (keys[SDL_SCANCODE_LEFT])
            p2x -= 300 * deltaTime;
        if (keys[SDL_SCANCODE_RIGHT])
            p2x += 300 * deltaTime;

        // Apply gravity player 1
        p1vy += gravity * deltaTime;
        p1y += p1vy * deltaTime;
        if (p1y >= groundY)
        {
            p1y = groundY;
            p1vy = 0;
            p1jumping = false;
        }

        // Apply gravity player 2
        p2vy += gravity * deltaTime;
        p2y += p2vy * deltaTime;
        if (p2y >= groundY)
        {
            p2y = groundY;
            p2vy = 0;
            p2jumping = false;
        }

        // Attack timers
        if (p1attacking)
        {
            p1attackTimer--;
            if (p1attackTimer <= 0)
                p1attacking = false;
        }
        if (p2attacking)
        {
            p2attackTimer--;
            if (p2attackTimer <= 0)
                p2attacking = false;
        }

        // Hitboxes
        SDL_FRect p1hitbox = {p1x + 50, p1y + 20, 60, 30};
        SDL_FRect p2hitbox = {p2x - 60, p2y + 20, 60, 30};
        SDL_FRect p1rect = {p1x, p1y, 50, 100};
        SDL_FRect p2rect = {p2x, p2y, 50, 100};

        // Collision detection
        if (p1attacking && SDL_HasRectIntersectionFloat(&p1hitbox, &p2rect))
        {
            p2health -= 0.5f;
            if (p2health < 0)
                p2health = 0;
        }
        if (p2attacking && SDL_HasRectIntersectionFloat(&p2hitbox, &p1rect))
        {
            p1health -= 0.5f;
            if (p1health < 0)
                p1health = 0;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw health bar backgrounds (grey)
        SDL_FRect p1healthBG = {50, 30, 200, 20};
        SDL_FRect p2healthBG = {550, 30, 200, 20};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &p1healthBG);
        SDL_RenderFillRect(renderer, &p2healthBG);

        // Draw health bars (green)
        SDL_FRect p1healthBar = {50, 30, (p1health / 100.0f) * 200, 20};
        SDL_FRect p2healthBar = {550, 30, (p2health / 100.0f) * 200, 20};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &p1healthBar);
        SDL_RenderFillRect(renderer, &p2healthBar);

        // Draw player 1 (red)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &p1rect);

        // Draw player 2 (blue)
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &p2rect);

        // Draw attack hitboxes (yellow) when attacking
        if (p1attacking)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p1hitbox);
        }
        if (p2attacking)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p2hitbox);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}