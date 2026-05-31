#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
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

    TTF_Font *font = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 48);
    if (!font)
    {
        std::cout << "Font failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Fighter Game", 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    SDL_Texture *p1sprite = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player1.png");
    SDL_Texture *p2sprite = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player2.png");

    if (!p1sprite || !p2sprite)
    {
        std::cout << "Sprite failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    float p1x = 100, p1y = 400;
    float p2x = 650, p2y = 400;
    float p1vy = 0, p2vy = 0;
    bool p1jumping = false, p2jumping = false;
    float p1health = 100, p2health = 100;
    bool p1attacking = false, p2attacking = false;
    int p1attackTimer = 0, p2attackTimer = 0;
    float gravity = 1500.0f;
    float jumpForce = -600.0f;
    float groundY = 400.0f;
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

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (gameOver && event.key.scancode == SDL_SCANCODE_SPACE)
                {
                    p1x = 100;
                    p1y = 400;
                    p2x = 650;
                    p2y = 400;
                    p1vy = 0;
                    p2vy = 0;
                    p1jumping = false;
                    p2jumping = false;
                    p1health = 100;
                    p2health = 100;
                    p1attacking = false;
                    p2attacking = false;
                    p1attackTimer = 0;
                    p2attackTimer = 0;
                    gameOver = false;
                    winner = "";
                }

                if (!gameOver)
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

            if (p1x < 0)
                p1x = 0;
            if (p1x > 750)
                p1x = 750;
            if (p2x < 0)
                p2x = 0;
            if (p2x > 750)
                p2x = 750;

            p1vy += gravity * deltaTime;
            p1y += p1vy * deltaTime;
            if (p1y >= groundY)
            {
                p1y = groundY;
                p1vy = 0;
                p1jumping = false;
            }

            p2vy += gravity * deltaTime;
            p2y += p2vy * deltaTime;
            if (p2y >= groundY)
            {
                p2y = groundY;
                p2vy = 0;
                p2jumping = false;
            }

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

            SDL_FRect p1hitbox = {p1x + 50, p1y + 20, 60, 30};
            SDL_FRect p2hitbox = {p2x - 60, p2y + 20, 60, 30};
            SDL_FRect p1rect = {p1x, p1y, 80, 100};
            SDL_FRect p2rect = {p2x, p2y, 80, 100};

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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_FRect ground = {0, 500, 800, 10};
        SDL_RenderFillRect(renderer, &ground);

        SDL_FRect p1healthBG = {50, 30, 200, 20};
        SDL_FRect p2healthBG = {550, 30, 200, 20};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &p1healthBG);
        SDL_RenderFillRect(renderer, &p2healthBG);

        SDL_FRect p1healthBar = {50, 30, (p1health / 100.0f) * 200, 20};
        SDL_FRect p2healthBar = {550, 30, (p2health / 100.0f) * 200, 20};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &p1healthBar);
        SDL_RenderFillRect(renderer, &p2healthBar);

        SDL_FRect p1rect = {p1x, p1y, 80, 100};
        SDL_FRect p2rect = {p2x, p2y, 80, 100};
        SDL_RenderTexture(renderer, p1sprite, NULL, &p1rect);
        SDL_RenderTexture(renderer, p2sprite, NULL, &p2rect);

        if (p1attacking)
        {
            SDL_FRect p1hitbox = {p1x + 80, p1y + 20, 60, 30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p1hitbox);
        }
        if (p2attacking)
        {
            SDL_FRect p2hitbox = {p2x - 60, p2y + 20, 60, 30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &p2hitbox);
        }

        if (gameOver)
        {
            SDL_Color white = {255, 255, 255, 255};

            // Line 1 - Winner
            SDL_Surface *surface1 = TTF_RenderText_Blended(font, winner.c_str(), 0, white);
            SDL_Texture *texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
            float text1W = surface1->w;
            float text1H = surface1->h;
            SDL_FRect textRect1 = {(800 - text1W) / 2, (600 - text1H) / 2 - 50, text1W, text1H};
            SDL_RenderTexture(renderer, texture1, NULL, &textRect1);
            SDL_DestroySurface(surface1);
            SDL_DestroyTexture(texture1);

            // Line 2 - Restart
            SDL_Surface *surface2 = TTF_RenderText_Blended(font, "Press SPACE to restart", 0, white);
            SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
            float text2W = surface2->w;
            float text2H = surface2->h;
            SDL_FRect textRect2 = {(800 - text2W) / 2, (600 - text2H) / 2 + 50, text2W, text2H};
            SDL_RenderTexture(renderer, texture2, NULL, &textRect2);
            SDL_DestroySurface(surface2);
            SDL_DestroyTexture(texture2);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(p1sprite);
    SDL_DestroyTexture(p2sprite);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}