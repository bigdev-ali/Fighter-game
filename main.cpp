#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>

enum GameState
{
    MENU,
    SHOW_ROUND,
    SHOW_FIGHT,
    PLAYING,
    ROUND_OVER,
    GAME_OVER
};

// ========================
// PLAYER CLASS
// ========================
struct Player
{
    float x, y;
    float vy;
    float health;
    bool jumping;
    bool attacking, kicking, blocking;
    int attackTimer, kickTimer;
    int flicker;
    SDL_Texture *sprite;
    std::string name;

    // Constructor — sets default values
    Player(std::string playerName, SDL_Texture *tex, float startX)
    {
        name = playerName;
        sprite = tex;
        x = startX;
        y = 400;
        vy = 0;
        health = 100;
        jumping = false;
        attacking = false;
        kicking = false;
        blocking = false;
        attackTimer = 0;
        kickTimer = 0;
        flicker = 0;
    }

    void reset(float startX)
    {
        x = startX;
        y = 400;
        vy = 0;
        health = 100;
        jumping = false;
        attacking = false;
        kicking = false;
        blocking = false;
        attackTimer = 0;
        kickTimer = 0;
        flicker = 0;
    }

    void update(float deltaTime, float gravity, float groundY)
    {
        // Apply gravity
        vy += gravity * deltaTime;
        y += vy * deltaTime;
        if (y >= groundY)
        {
            y = groundY;
            vy = 0;
            jumping = false;
        }

        // Attack timers
        if (attacking)
        {
            attackTimer--;
            if (attackTimer <= 0)
                attacking = false;
        }
        if (kicking)
        {
            kickTimer--;
            if (kickTimer <= 0)
                kicking = false;
        }

        // Flicker countdown
        if (flicker > 0)
            flicker--;
    }

    void draw(SDL_Renderer *renderer)
    {
        SDL_FRect rect = {x, y, 80, 100};

        if (blocking)
            SDL_SetTextureColorMod(sprite, 50, 50, 255);
        else if (flicker > 0 && flicker % 2 == 0)
            SDL_SetTextureColorMod(sprite, 255, 50, 50);
        else
            SDL_SetTextureColorMod(sprite, 255, 255, 255);

        SDL_RenderTexture(renderer, sprite, NULL, &rect);
    }

    void takeDamage(float amount)
    {
        health -= amount;
        if (health < 0)
            health = 0;
        flicker = 10;
    }

    SDL_FRect getRect() { return {x, y, 80, 100}; }
    SDL_FRect getPunchHitbox(bool facingRight)
    {
        if (facingRight)
            return {x + 80, y + 20, 60, 30};
        else
            return {x - 80, y + 20, 60, 30};
    }
    SDL_FRect getKickHitbox(bool facingRight)
    {
        if (facingRight)
            return {x + 80, y + 60, 80, 30};
        else
            return {x - 80, y + 60, 80, 30};
    }
};
// MAIN
int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL failed to init: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!TTF_Init())
    {
        std::cout << "TTF failed to init: " << SDL_GetError() << std::endl;
        return 1;
    }

    TTF_Font *fontBig = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 80);
    TTF_Font *fontSmall = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 28);
    TTF_Font *fontMed = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 48);
    TTF_Font *fontTitle = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 90);
    TTF_Font *fontKO = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 120);
    TTF_Font *fontTimer = TTF_OpenFont("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\times.ttf", 36);

    if (!fontBig || !fontSmall || !fontMed || !fontTitle || !fontKO || !fontTimer)
    {
        std::cout << "Font failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("King of Goon", 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    SDL_Texture *background = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\background.png");
    SDL_Texture *p1tex = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player1.png");
    SDL_Texture *p2tex = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player2.png");

    if (!background || !p1tex || !p2tex)
    {
        std::cout << "Image failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Audio
    SDL_AudioSpec themeSpec, hitSpec;
    Uint8 *themeData, *hitData;
    Uint32 themeLen, hitLen;

    SDL_LoadWAV("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\theme.wav", &themeSpec, &themeData, &themeLen);
    SDL_LoadWAV("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\hit.wav", &hitSpec, &hitData, &hitLen);

    SDL_AudioStream *themeStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &themeSpec, NULL, NULL);
    SDL_AudioStream *hitStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &hitSpec, NULL, NULL);

    auto playTheme = [&]()
    {
        SDL_ClearAudioStream(themeStream);
        for (int i = 0; i < 10; i++)
            SDL_PutAudioStreamData(themeStream, themeData, themeLen);
        SDL_ResumeAudioStreamDevice(themeStream);
    };

    auto playHit = [&]()
    {
        SDL_ClearAudioStream(hitStream);
        SDL_PutAudioStreamData(hitStream, hitData, hitLen);
        SDL_ResumeAudioStreamDevice(hitStream);
    };

    auto stopTheme = [&]()
    {
        SDL_ClearAudioStream(themeStream);
        SDL_PauseAudioStreamDevice(themeStream);
    };

    // Create players
    Player p1("Player 1", p1tex, 100);
    Player p2("Player 2", p2tex, 650);

    // Game state
    float gravity = 1500.0f;
    float jumpForce = -600.0f;
    float groundY = 400.0f;
    int currentRound = 1;
    int p1wins = 0, p2wins = 0;
    std::string winner = "";
    GameState state = MENU;
    float stateTimer = 1.5f;
    float roundTimer = 60.0f;

    SDL_FRect btn2P = {250, 280, 300, 70};
    SDL_FRect btnVSComp = {250, 380, 300, 70};

    auto drawButton = [&](SDL_FRect rect, const char *text, bool hovered)
    {
        if (hovered)
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        else
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &rect);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface *surf = TTF_RenderText_Blended(fontMed, text, 0, white);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect textRect = {
            rect.x + (rect.w - surf->w) / 2,
            rect.y + (rect.h - surf->h) / 2,
            (float)surf->w,
            (float)surf->h};
        SDL_RenderTexture(renderer, tex, NULL, &textRect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto drawText = [&](TTF_Font *font, const char *text, SDL_Color color, float x, float y)
    {
        SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect rect = {x, y, (float)surf->w, (float)surf->h};
        SDL_RenderTexture(renderer, tex, NULL, &rect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto drawTextCentered = [&](TTF_Font *font, const char *text, SDL_Color color, float y)
    {
        SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect rect = {(800 - (float)surf->w) / 2, y, (float)surf->w, (float)surf->h};
        SDL_RenderTexture(renderer, tex, NULL, &rect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto resetGame = [&]()
    {
        p1.reset(100);
        p2.reset(650);
        currentRound = 1;
        p1wins = 0;
        p2wins = 0;
        winner = "";
        stateTimer = 1.5f;
        roundTimer = 60.0f;
    };

    auto resetRound = [&]()
    {
        p1.reset(100);
        p2.reset(650);
        stateTimer = 1.5f;
        roundTimer = 60.0f;
    };

    Uint64 lastTime = SDL_GetTicks();
    bool running = true;
    SDL_Event event;

    while (running)
    {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        bool hover2P = (mouseX >= btn2P.x && mouseX <= btn2P.x + btn2P.w &&
                        mouseY >= btn2P.y && mouseY <= btn2P.y + btn2P.h);
        bool hoverVSComp = (mouseX >= btnVSComp.x && mouseX <= btnVSComp.x + btnVSComp.w &&
                            mouseY >= btnVSComp.y && mouseY <= btnVSComp.y + btnVSComp.h);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                if (state == MENU && hover2P)
                {
                    resetGame();
                    state = SHOW_ROUND;
                    playTheme();
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (state == GAME_OVER && event.key.scancode == SDL_SCANCODE_SPACE)
                {
                    resetGame();
                    state = MENU;
                    stopTheme();
                }

                if (state == PLAYING)
                {
                    // P1 jump
                    if (event.key.scancode == SDL_SCANCODE_W && !p1.jumping)
                    {
                        p1.vy = jumpForce;
                        p1.jumping = true;
                    }
                    // P2 jump
                    if (event.key.scancode == SDL_SCANCODE_UP && !p2.jumping)
                    {
                        p2.vy = jumpForce;
                        p2.jumping = true;
                    }
                    // P1 punch
                    if (event.key.scancode == SDL_SCANCODE_F && !p1.attacking && !p1.blocking && !p1.kicking)
                    {
                        p1.attacking = true;
                        p1.attackTimer = 15;
                    }
                    // P2 punch
                    if (event.key.scancode == SDL_SCANCODE_K && !p2.attacking && !p2.blocking && !p2.kicking)
                    {
                        p2.attacking = true;
                        p2.attackTimer = 15;
                    }
                    // P1 kick
                    if (event.key.scancode == SDL_SCANCODE_G && !p1.kicking && !p1.blocking && !p1.attacking)
                    {
                        p1.kicking = true;
                        p1.kickTimer = 20;
                    }
                    // P2 kick
                    if (event.key.scancode == SDL_SCANCODE_L && !p2.kicking && !p2.blocking && !p2.attacking)
                    {
                        p2.kicking = true;
                        p2.kickTimer = 20;
                    }
                }
            }
        }

        // Blocking — held keys
        if (state == PLAYING)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);
            p1.blocking = keys[SDL_SCANCODE_S];
            p2.blocking = keys[SDL_SCANCODE_DOWN];
        }

        // State machine
        if (state == SHOW_ROUND)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0)
            {
                state = SHOW_FIGHT;
                stateTimer = 1.0f;
            }
        }
        else if (state == SHOW_FIGHT)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0)
                state = PLAYING;
        }
        else if (state == PLAYING)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);

            // Movement
            if (!p1.blocking)
            {
                if (keys[SDL_SCANCODE_A])
                    p1.x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_D])
                    p1.x += 300 * deltaTime;
            }
            if (!p2.blocking)
            {
                if (keys[SDL_SCANCODE_LEFT])
                    p2.x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_RIGHT])
                    p2.x += 300 * deltaTime;
            }

            // Boundaries
            if (p1.x < 0)
                p1.x = 0;
            if (p1.x > 750)
                p1.x = 750;
            if (p2.x < 0)
                p2.x = 0;
            if (p2.x > 750)
                p2.x = 750;

            // Update both players
            p1.update(deltaTime, gravity, groundY);
            p2.update(deltaTime, gravity, groundY);

            // Round timer
            roundTimer -= deltaTime;
            if (roundTimer <= 0)
            {
                roundTimer = 0;
                if (p1.health > p2.health)
                    p1wins++;
                else if (p2.health > p1.health)
                    p2wins++;

                if (p1wins == 2)
                {
                    winner = "Player 1 Wins!";
                    state = GAME_OVER;
                    stopTheme();
                }
                else if (p2wins == 2)
                {
                    winner = "Player 2 Wins!";
                    state = GAME_OVER;
                    stopTheme();
                }
                else
                {
                    currentRound++;
                    state = ROUND_OVER;
                    stateTimer = 2.0f;
                }
            }

            // Hitboxes
            SDL_FRect p1rect = p1.getRect();
            SDL_FRect p2rect = p2.getRect();
            SDL_FRect p1punch = p1.getPunchHitbox(true);
            SDL_FRect p2punch = p2.getPunchHitbox(false);
            SDL_FRect p1kick = p1.getKickHitbox(true);
            SDL_FRect p2kick = p2.getKickHitbox(false);

            // P1 punch hits P2
            if (p1.attacking && SDL_HasRectIntersectionFloat(&p1punch, &p2rect))
            {
                if (p2.blocking)
                {
                    p2.health -= 0.1f;
                    if (p2.health < 0)
                        p2.health = 0;
                }
                else
                {
                    p2.takeDamage(0.5f);
                    playHit();
                }
            }
            // P2 punch hits P1
            if (p2.attacking && SDL_HasRectIntersectionFloat(&p2punch, &p1rect))
            {
                if (p1.blocking)
                {
                    p1.health -= 0.1f;
                    if (p1.health < 0)
                        p1.health = 0;
                }
                else
                {
                    p1.takeDamage(0.5f);
                    playHit();
                }
            }
            // P1 kick hits P2
            if (p1.kicking && SDL_HasRectIntersectionFloat(&p1kick, &p2rect))
            {
                if (p2.blocking)
                {
                    p2.health -= 0.15f;
                    if (p2.health < 0)
                        p2.health = 0;
                }
                else
                {
                    p2.takeDamage(0.7f);
                    playHit();
                }
            }
            // P2 kick hits P1
            if (p2.kicking && SDL_HasRectIntersectionFloat(&p2kick, &p1rect))
            {
                if (p1.blocking)
                {
                    p1.health -= 0.15f;
                    if (p1.health < 0)
                        p1.health = 0;
                }
                else
                {
                    p1.takeDamage(0.7f);
                    playHit();
                }
            }

            // Check round over
            if (p1.health <= 0 || p2.health <= 0)
            {
                if (p1.health <= 0)
                    p2wins++;
                if (p2.health <= 0)
                    p1wins++;

                if (p1wins == 2)
                {
                    winner = "Player 1 Wins!";
                    state = GAME_OVER;
                    stopTheme();
                }
                else if (p2wins == 2)
                {
                    winner = "Player 2 Wins!";
                    state = GAME_OVER;
                    stopTheme();
                }
                else
                {
                    currentRound++;
                    state = ROUND_OVER;
                    stateTimer = 2.0f;
                }
            }
        }
        else if (state == ROUND_OVER)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0)
            {
                resetRound();
                currentRound;
                state = SHOW_ROUND;
            }
        }

        // ========================
        // DRAW
        // ========================
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold = {255, 215, 0, 255};
        SDL_Color red = {255, 0, 0, 255};
        SDL_Color black = {0, 0, 0, 255};
        SDL_Color yellow = {255, 215, 0, 255};

        if (state == MENU)
        {
            drawTextCentered(fontTitle, "KING OF GOON", gold, 100);
            drawButton(btn2P, "2 Player", hover2P);
            drawButton(btnVSComp, "VS Computer", hoverVSComp);
        }
        else
        {
            // Background
            SDL_FRect bgRect = {0, 0, 800, 600};
            SDL_RenderTexture(renderer, background, NULL, &bgRect);

            // Ground
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_FRect ground = {0, 500, 800, 10};
            SDL_RenderFillRect(renderer, &ground);

            // Health bar backgrounds
            SDL_FRect p1healthBG = {50, 50, 200, 20};
            SDL_FRect p2healthBG = {550, 50, 200, 20};
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &p1healthBG);
            SDL_RenderFillRect(renderer, &p2healthBG);

            // Health bars
            SDL_FRect p1healthBar = {50, 50, (p1.health / 100.0f) * 200, 20};
            SDL_FRect p2healthBar = {550, 50, (p2.health / 100.0f) * 200, 20};
            SDL_SetRenderDrawColor(renderer, p1.health < 30 ? 255 : 0, p1.health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p1healthBar);
            SDL_SetRenderDrawColor(renderer, p2.health < 30 ? 255 : 0, p2.health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p2healthBar);

            // Player names
            drawText(fontSmall, p1.name.c_str(), white, 50, 5);
            drawText(fontSmall, p2.name.c_str(), white, 550, 5);

            // Round wins dots
            for (int i = 0; i < p1wins; i++)
            {
                SDL_FRect dot = {50.0f + i * 20, 75, 15, 15};
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_RenderFillRect(renderer, &dot);
            }
            for (int i = 0; i < p2wins; i++)
            {
                SDL_FRect dot = {550.0f + i * 20, 75, 15, 15};
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_RenderFillRect(renderer, &dot);
            }

            // Timer
            int timerSeconds = (int)roundTimer;
            SDL_Color timerColor = timerSeconds <= 10 ? red : white;
            drawTextCentered(fontTimer, std::to_string(timerSeconds).c_str(), timerColor, 40);

            // Draw players
            p1.draw(renderer);
            p2.draw(renderer);

            // State overlays
            if (state == SHOW_ROUND)
            {
                std::string roundText = "Round " + std::to_string(currentRound);
                drawTextCentered(fontBig, roundText.c_str(), white, (600 - 80) / 2);
            }
            else if (state == SHOW_FIGHT)
            {
                drawTextCentered(fontBig, "FIGHT!", yellow, (600 - 80) / 2);
            }
            else if (state == ROUND_OVER)
            {
                drawTextCentered(fontKO, "K.O!", black, (600 - 120) / 2);
            }
            else if (state == GAME_OVER)
            {
                drawTextCentered(fontBig, winner.c_str(), white, (600 - 80) / 2 - 60);
                drawTextCentered(fontMed, "Press SPACE to go to menu", white, (600 - 48) / 2 + 60);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_free(themeData);
    SDL_free(hitData);
    SDL_DestroyAudioStream(themeStream);
    SDL_DestroyAudioStream(hitStream);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(p1tex);
    SDL_DestroyTexture(p2tex);
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontMed);
    TTF_CloseFont(fontTitle);
    TTF_CloseFont(fontKO);
    TTF_CloseFont(fontTimer);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}