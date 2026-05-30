#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL failed to init: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!TTF_Init())
    {
        std::cout << "TTF failed to init: " << SDL_GetError() << std::endl;
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 64);
    if (!font)
    {
        std::cout << "Font failed to load: " << SDL_GetError() << std::endl;
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

    // Game state
    bool gameOver = false;
    std::string winner = "";

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
                running = false;

            if (!gameOver && event.type == SDL_EVENT_KEY_DOWN)
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

        if (!gameOver)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);

            if (keys[SDL_SCANCODE_A])
                p1x -= 300 * deltaTime;
            if (keys[SDL_SCANCODE_D])
                p1x += 300 * deltaTime;
            if (keys[SDL_SCANCODE_LEFT])
                p2x -= 300 * deltaTime;
            if (keys[SDL_SCANCODE_RIGHT])
                p2x += 300 * deltaTime;

            // Boundaries
            if (p1x < 0)
                p1x = 0;
            if (p1x > 750)
                p1x = 750;
            if (p2x < 0)
                p2x = 0;
            if (p2x > 750)
                p2x = 750;

            // Gravity player 1
            p1vy += gravity * deltaTime;
            p1y += p1vy * deltaTime;
            if (p1y >= groundY)
            {
                p1y = groundY;
                p1vy = 0;
                p1jumping = false;
            }

            // Gravity player 2
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

            // Collision
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

            // Check game over
            if (p1health <= 0)
            {
                gameOver = true;
                winner = "Player 2 Wins!";
            }
            if (p2health <= 0)
            {
                gameOver = true;
                winner = "Player 1 Wins!";
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw ground
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect ground = {0, 500, 800, 10};
        SDL_RenderFillRect(renderer, &ground);

        // Health bar backgrounds
        SDL_FRect p1healthBG = {50, 30, 200, 20};
        SDL_FRect p2healthBG = {550, 30, 200, 20};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &p1healthBG);
        SDL_RenderFillRect(renderer, &p2healthBG);

        // Health bars
        SDL_FRect p1healthBar = {50, 30, (p1health / 100.0f) * 200, 20};
        SDL_FRect p2healthBar = {550, 30, (p2health / 100.0f) * 200, 20};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &p1healthBar);
        SDL_RenderFillRect(renderer, &p2healthBar);

        // Draw players
        SDL_FRect p1rect = {p1x, p1y, 50, 100};
        SDL_FRect p2rect = {p2x, p2y, 50, 100};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &p1rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &p2rect);

        // Draw hitboxes
        if (p1attacking)
        {
            SDL_FRect p1hitbox = {p1x + 50, p1y + 20, 60, 30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p1hitbox);
        }
        if (p2attacking)
        {
            SDL_FRect p2hitbox = {p2x - 60, p2y + 20, 60, 30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p2hitbox);
        }

        // Game over screen
        if (gameOver)
        {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surface = TTF_RenderText_Blended(font, winner.c_str(), 0, white);
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

            float textW = surface->w;
            float textH = surface->h;
            SDL_FRect textRect = {(800 - textW) / 2, (600 - textH) / 2, textW, textH};

            SDL_RenderTexture(renderer, texture, NULL, &textRect);
            SDL_DestroySurface(surface);
            SDL_DestroyTexture(texture);
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}