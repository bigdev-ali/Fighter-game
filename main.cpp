#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

enum GameState
{
    MENU,
    SHOW_ROUND,
    SHOW_FIGHT,
    PLAYING,
    ROUND_OVER,
    GAME_OVER
};

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

    if (!fontBig || !fontSmall || !fontMed || !fontTitle || !fontKO)
    {
        std::cout << "Font failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("King of Goon", 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    SDL_Texture *background = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\background.png");
    if (!background)
    {
        std::cout << "Background failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Texture *p1sprite = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player1.png");
    SDL_Texture *p2sprite = IMG_LoadTexture(renderer, "C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\player2.png");

    if (!p1sprite || !p2sprite)
    {
        std::cout << "Sprite failed to load: " << SDL_GetError() << std::endl;
        return 1;
    }

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

    float p1x = 100, p1y = 400;
    float p2x = 650, p2y = 400;
    float p1vy = 0, p2vy = 0;
    bool p1jumping = false, p2jumping = false;
    float p1health = 100, p2health = 100;
    bool p1attacking = false, p2attacking = false;
    int p1attackTimer = 0, p2attackTimer = 0;
    int p1flicker = 0, p2flicker = 0;
    bool p1blocking = false, p2blocking = false;
    float gravity = 1500.0f;
    float jumpForce = -600.0f;
    float groundY = 400.0f;
    int currentRound = 1;
    int p1wins = 0, p2wins = 0;
    std::string winner = "";
    GameState state = MENU;
    float stateTimer = 1.5f;

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

    auto resetGame = [&]()
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
        p1flicker = 0;
        p2flicker = 0;
        p1blocking = false;
        p2blocking = false;
        currentRound = 1;
        p1wins = 0;
        p2wins = 0;
        winner = "";
        stateTimer = 1.5f;
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
                    if (event.key.scancode == SDL_SCANCODE_F && !p1attacking && !p1blocking)
                    {
                        p1attacking = true;
                        p1attackTimer = 15;
                    }
                    if (event.key.scancode == SDL_SCANCODE_K && !p2attacking && !p2blocking)
                    {
                        p2attacking = true;
                        p2attackTimer = 15;
                    }
                }
            }
        }

        if (state == PLAYING)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);
            p1blocking = keys[SDL_SCANCODE_S];
            p2blocking = keys[SDL_SCANCODE_DOWN];
        }

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

            if (!p1blocking)
            {
                if (keys[SDL_SCANCODE_A])
                    p1x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_D])
                    p1x += 300 * deltaTime;
            }
            if (!p2blocking)
            {
                if (keys[SDL_SCANCODE_LEFT])
                    p2x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_RIGHT])
                    p2x += 300 * deltaTime;
            }

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

            if (p1flicker > 0)
                p1flicker--;
            if (p2flicker > 0)
                p2flicker--;

            SDL_FRect p1hitbox = {p1x + 50, p1y + 20, 60, 30};
            SDL_FRect p2hitbox = {p2x - 60, p2y + 20, 60, 30};
            SDL_FRect p1rect = {p1x, p1y, 80, 100};
            SDL_FRect p2rect = {p2x, p2y, 80, 100};

            if (p1attacking && SDL_HasRectIntersectionFloat(&p1hitbox, &p2rect))
            {
                if (p2blocking)
                {
                    p2health -= 0.1f;
                }
                else
                {
                    p2health -= 0.5f;
                    p2flicker = 10;
                    playHit();
                }
                if (p2health < 0)
                    p2health = 0;
            }

            if (p2attacking && SDL_HasRectIntersectionFloat(&p2hitbox, &p1rect))
            {
                if (p1blocking)
                {
                    p1health -= 0.1f;
                }
                else
                {
                    p1health -= 0.5f;
                    p1flicker = 10;
                    playHit();
                }
                if (p1health < 0)
                    p1health = 0;
            }

            if (p1health <= 0 || p2health <= 0)
            {
                if (p1health <= 0)
                    p2wins++;
                if (p2health <= 0)
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
                p1flicker = 0;
                p2flicker = 0;
                p1blocking = false;
                p2blocking = false;
                state = SHOW_ROUND;
                stateTimer = 1.5f;
            }
        }

        // DRAW
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (state == MENU)
        {
            SDL_Color gold = {255, 215, 0, 255};
            SDL_Surface *titleSurf = TTF_RenderText_Blended(fontTitle, "KING OF GOON", 0, gold);
            SDL_Texture *titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
            SDL_FRect titleRect = {(800 - (float)titleSurf->w) / 2, 100, (float)titleSurf->w, (float)titleSurf->h};
            SDL_RenderTexture(renderer, titleTex, NULL, &titleRect);
            SDL_DestroySurface(titleSurf);
            SDL_DestroyTexture(titleTex);

            drawButton(btn2P, "2 Player", hover2P);
            drawButton(btnVSComp, "VS Computer", hoverVSComp);
        }
        else
        {
            SDL_FRect bgRect = {0, 0, 800, 600};
            SDL_RenderTexture(renderer, background, NULL, &bgRect);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_FRect ground = {0, 500, 800, 10};
            SDL_RenderFillRect(renderer, &ground);

            SDL_FRect p1healthBG = {50, 50, 200, 20};
            SDL_FRect p2healthBG = {550, 50, 200, 20};
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &p1healthBG);
            SDL_RenderFillRect(renderer, &p2healthBG);

            SDL_FRect p1healthBar = {50, 50, (p1health / 100.0f) * 200, 20};
            SDL_FRect p2healthBar = {550, 50, (p2health / 100.0f) * 200, 20};
            SDL_SetRenderDrawColor(renderer, p1health < 30 ? 255 : 0, p1health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p1healthBar);
            SDL_SetRenderDrawColor(renderer, p2health < 30 ? 255 : 0, p2health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p2healthBar);

            SDL_Color white = {255, 255, 255, 255};

            SDL_Surface *name1surf = TTF_RenderText_Blended(fontSmall, "Player 1", 0, white);
            SDL_Texture *name1tex = SDL_CreateTextureFromSurface(renderer, name1surf);
            SDL_FRect name1rect = {50, 5, (float)name1surf->w, (float)name1surf->h};
            SDL_RenderTexture(renderer, name1tex, NULL, &name1rect);
            SDL_DestroySurface(name1surf);
            SDL_DestroyTexture(name1tex);

            SDL_Surface *name2surf = TTF_RenderText_Blended(fontSmall, "Player 2", 0, white);
            SDL_Texture *name2tex = SDL_CreateTextureFromSurface(renderer, name2surf);
            SDL_FRect name2rect = {550, 5, (float)name2surf->w, (float)name2surf->h};
            SDL_RenderTexture(renderer, name2tex, NULL, &name2rect);
            SDL_DestroySurface(name2surf);
            SDL_DestroyTexture(name2tex);

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

            SDL_FRect p1rect = {p1x, p1y, 80, 100};
            SDL_FRect p2rect = {p2x, p2y, 80, 100};

            if (p1blocking)
                SDL_SetTextureColorMod(p1sprite, 50, 50, 255);
            else if (p1flicker > 0 && p1flicker % 2 == 0)
                SDL_SetTextureColorMod(p1sprite, 255, 50, 50);
            else
                SDL_SetTextureColorMod(p1sprite, 255, 255, 255);
            SDL_RenderTexture(renderer, p1sprite, NULL, &p1rect);

            if (p2blocking)
                SDL_SetTextureColorMod(p2sprite, 50, 50, 255);
            else if (p2flicker > 0 && p2flicker % 2 == 0)
                SDL_SetTextureColorMod(p2sprite, 255, 50, 50);
            else
                SDL_SetTextureColorMod(p2sprite, 255, 255, 255);
            SDL_RenderTexture(renderer, p2sprite, NULL, &p2rect);

            if (state == SHOW_ROUND)
            {
                std::string roundText = "Round " + std::to_string(currentRound);
                SDL_Surface *rsurf = TTF_RenderText_Blended(fontBig, roundText.c_str(), 0, white);
                SDL_Texture *rtex = SDL_CreateTextureFromSurface(renderer, rsurf);
                SDL_FRect rrect = {(800 - (float)rsurf->w) / 2, (600 - (float)rsurf->h) / 2, (float)rsurf->w, (float)rsurf->h};
                SDL_RenderTexture(renderer, rtex, NULL, &rrect);
                SDL_DestroySurface(rsurf);
                SDL_DestroyTexture(rtex);
            }
            else if (state == SHOW_FIGHT)
            {
                SDL_Color yellow = {255, 215, 0, 255};
                SDL_Surface *fsurf = TTF_RenderText_Blended(fontBig, "FIGHT!", 0, yellow);
                SDL_Texture *ftex = SDL_CreateTextureFromSurface(renderer, fsurf);
                SDL_FRect frect = {(800 - (float)fsurf->w) / 2, (600 - (float)fsurf->h) / 2, (float)fsurf->w, (float)fsurf->h};
                SDL_RenderTexture(renderer, ftex, NULL, &frect);
                SDL_DestroySurface(fsurf);
                SDL_DestroyTexture(ftex);
            }
            else if (state == ROUND_OVER)
            {
                SDL_Color black = {0, 0, 0, 255};
                SDL_Surface *kosurf = TTF_RenderText_Blended(fontKO, "K.O!", 0, black);
                SDL_Texture *kotex = SDL_CreateTextureFromSurface(renderer, kosurf);
                SDL_FRect korect = {(800 - (float)kosurf->w) / 2, (600 - (float)kosurf->h) / 2, (float)kosurf->w, (float)kosurf->h};
                SDL_RenderTexture(renderer, kotex, NULL, &korect);
                SDL_DestroySurface(kosurf);
                SDL_DestroyTexture(kotex);
            }
            else if (state == GAME_OVER)
            {
                SDL_Surface *gsurf = TTF_RenderText_Blended(fontBig, winner.c_str(), 0, white);
                SDL_Texture *gtex = SDL_CreateTextureFromSurface(renderer, gsurf);
                SDL_FRect grect = {(800 - (float)gsurf->w) / 2, (600 - (float)gsurf->h) / 2 - 60, (float)gsurf->w, (float)gsurf->h};
                SDL_RenderTexture(renderer, gtex, NULL, &grect);
                SDL_DestroySurface(gsurf);
                SDL_DestroyTexture(gtex);

                SDL_Surface *rsurf = TTF_RenderText_Blended(fontMed, "Press SPACE to go to menu", 0, white);
                SDL_Texture *rtex = SDL_CreateTextureFromSurface(renderer, rsurf);
                SDL_FRect rrect = {(800 - (float)rsurf->w) / 2, (600 - (float)rsurf->h) / 2 + 60, (float)rsurf->w, (float)rsurf->h};
                SDL_RenderTexture(renderer, rtex, NULL, &rrect);
                SDL_DestroySurface(rsurf);
                SDL_DestroyTexture(rtex);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_free(themeData);
    SDL_free(hitData);
    SDL_DestroyAudioStream(themeStream);
    SDL_DestroyAudioStream(hitStream);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(p1sprite);
    SDL_DestroyTexture(p2sprite);
    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontMed);
    TTF_CloseFont(fontTitle);
    TTF_CloseFont(fontKO);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}