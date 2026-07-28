#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

enum GameState { MENU, CONTROLS, ENTER_NAMES, SHOW_ROUND, SHOW_FIGHT, PLAYING, PAUSED, ROUND_OVER, GAME_OVER };
enum AIState { AI_APPROACH, AI_ATTACK, AI_BLOCK, AI_RETREAT };

struct Projectile {
    float x, y;
    float vx;
    bool active;
    bool fromP1;

    Projectile()
    {
        active = false;
        x = 0; y = 0;
        vx = 0;
        fromP1 = true;
    }

    void fire(float startX, float startY, bool goingRight, bool p1)
    {
        x = startX;
        y = startY;
        vx = goingRight ? 900.0f : -900.0f;
        active = true;
        fromP1 = p1;
    }

    void update(float deltaTime)
    {
        if (!active) return;
        x += vx * deltaTime;
        if (x < 0 || x > 800) active = false;
    }

    SDL_FRect getRect() { return {x, y, 20, 20}; }
};

struct Player {
    float x, y;
    float vy;
    float vx;
    float health;
    float displayHealth;
    bool jumping;
    bool hasDoubleJumped;
    bool attacking, kicking, blocking, taunting, grabbing;
    bool airAttacking;
    bool charging;
    float chargeTimer;
    float chargeMaxTime;
    int attackTimer, kickTimer, grabTimer;
    int airAttackTimer;
    int flicker;
    SDL_Texture *sprite;
    std::string name;

    Player(std::string playerName, SDL_Texture *tex, float startX)
    {
        name = playerName;
        sprite = tex;
        x = startX;
        y = 400;
        vy = 0;
        vx = 0;
        health = 100;
        displayHealth = 100;
        jumping = false;
        hasDoubleJumped = false;
        attacking = false;
        kicking = false;
        blocking = false;
        taunting = false;
        grabbing = false;
        airAttacking = false;
        charging = false;
        chargeTimer = 0;
        chargeMaxTime = 1.5f;
        attackTimer = 0;
        kickTimer = 0;
        grabTimer = 0;
        airAttackTimer = 0;
        flicker = 0;
    }

    void reset(float startX)
    {
        x = startX;
        y = 400;
        vy = 0;
        vx = 0;
        health = 100;
        displayHealth = 100;
        jumping = false;
        hasDoubleJumped = false;
        attacking = false;
        kicking = false;
        blocking = false;
        taunting = false;
        grabbing = false;
        airAttacking = false;
        charging = false;
        chargeTimer = 0;
        attackTimer = 0;
        kickTimer = 0;
        grabTimer = 0;
        airAttackTimer = 0;
        flicker = 0;
    }

    void update(float deltaTime, float gravity, float groundY)
    {
        x += vx * deltaTime;
        if (vx > 0) { vx -= 800 * deltaTime; if (vx < 0) vx = 0; }
        if (vx < 0) { vx += 800 * deltaTime; if (vx > 0) vx = 0; }

        vy += gravity * deltaTime;
        y += vy * deltaTime;
        if (y >= groundY)
        {
            y = groundY;
            vy = 0;
            jumping = false;
            hasDoubleJumped = false;
            airAttacking = false;
        }

        if (attacking) { attackTimer--; if (attackTimer <= 0) attacking = false; }
        if (kicking) { kickTimer--; if (kickTimer <= 0) kicking = false; }
        if (grabbing) { grabTimer--; if (grabTimer <= 0) grabbing = false; }
        if (airAttacking) { airAttackTimer--; if (airAttackTimer <= 0) airAttacking = false; }
        if (flicker > 0) flicker--;

        if (taunting && health < 100)
        {
            health += 0.2f * deltaTime;
            if (health > 100) health = 100;
        }

        if (displayHealth > health)
        {
            displayHealth -= 15.0f * deltaTime;
            if (displayHealth < health) displayHealth = health;
        }
    }

    void draw(SDL_Renderer *renderer)
    {
        SDL_FRect rect = {x, y, 80, 100};
        if (charging)
            SDL_SetTextureColorMod(sprite, 255, 255, 50);
        else if (taunting)
            SDL_SetTextureColorMod(sprite, 255, 215, 0);
        else if (blocking)
            SDL_SetTextureColorMod(sprite, 50, 50, 255);
        else if (flicker > 0 && flicker % 2 == 0)
            SDL_SetTextureColorMod(sprite, 255, 50, 50);
        else
            SDL_SetTextureColorMod(sprite, 255, 255, 255);
        SDL_RenderTexture(renderer, sprite, NULL, &rect);

        if (charging)
        {
            SDL_FRect chargeBG = {x, y - 15, 80, 8};
            SDL_FRect chargeBar = {x, y - 15, (chargeTimer / chargeMaxTime) * 80, 8};
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_RenderFillRect(renderer, &chargeBG);
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &chargeBar);
        }
    }

    void takeDamage(float amount)
    {
        health -= amount;
        if (health < 0) health = 0;
        flicker = 10;
        taunting = false;
        charging = false;
        chargeTimer = 0;
    }

    void getThrown(bool thrownRight)
    {
        vy = -600.0f;
        vx = thrownRight ? 800.0f : -800.0f;
        jumping = true;
        taunting = false;
        charging = false;
        chargeTimer = 0;
        flicker = 15;
    }

    bool isFullyCharged() { return chargeTimer >= chargeMaxTime; }

    SDL_FRect getRect() { return {x, y, 80, 100}; }
    SDL_FRect getPunchHitbox(bool facingRight)
    {
        if (facingRight) return {x + 80, y + 20, 60, 30};
        else return {x - 80, y + 20, 60, 30};
    }
    SDL_FRect getKickHitbox(bool facingRight)
    {
        if (facingRight) return {x + 80, y + 60, 80, 30};
        else return {x - 80, y + 60, 80, 30};
    }
    SDL_FRect getGrabHitbox(bool facingRight)
    {
        if (facingRight) return {x + 80, y + 10, 50, 80};
        else return {x - 50, y + 10, 50, 80};
    }
    SDL_FRect getAirAttackHitbox(bool facingRight)
    {
        if (facingRight) return {x + 60, y + 80, 80, 40};
        else return {x - 80, y + 80, 80, 40};
    }
};

struct AI {
    AIState state;
    float actionTimer;
    float actionInterval;
    float attackRange;
    float retreatRange;

    AI()
    {
        state = AI_APPROACH;
        actionTimer = 0;
        actionInterval = 0.5f;
        attackRange = 120.0f;
        retreatRange = 60.0f;
    }

    void reset()
    {
        state = AI_APPROACH;
        actionTimer = 0;
    }

    void update(Player &ai, Player &player, float deltaTime)
    {
        actionTimer -= deltaTime;
        float dist = ai.x - player.x;
        float absDist = dist < 0 ? -dist : dist;
        bool playerToRight = player.x > ai.x;

        if (ai.blocking && !player.attacking && !player.kicking && !player.grabbing)
            ai.blocking = false;

        if (actionTimer <= 0)
        {
            actionTimer = actionInterval;
            int roll = rand() % 100;

            if (absDist > attackRange) state = AI_APPROACH;
            else if (absDist < retreatRange)
            {
                if (roll < 40) state = AI_RETREAT;
                else state = AI_ATTACK;
            }
            else
            {
                if (player.attacking || player.kicking)
                {
                    if (roll < 50) state = AI_BLOCK;
                    else state = AI_ATTACK;
                }
                else
                {
                    if (roll < 70) state = AI_ATTACK;
                    else state = AI_BLOCK;
                }
            }
        }

        ai.blocking = false;

        if (state == AI_APPROACH)
        {
            if (playerToRight) ai.x += 200 * deltaTime;
            else ai.x -= 200 * deltaTime;
        }
        else if (state == AI_RETREAT)
        {
            if (playerToRight) ai.x -= 150 * deltaTime;
            else ai.x += 150 * deltaTime;
        }
        else if (state == AI_BLOCK)
        {
            ai.blocking = true;
        }
        else if (state == AI_ATTACK)
        {
            if (absDist <= attackRange && !ai.attacking && !ai.kicking && !ai.grabbing)
            {
                int attackRoll = rand() % 3;
                if (attackRoll == 0) { ai.attacking = true; ai.attackTimer = 15; }
                else if (attackRoll == 1) { ai.kicking = true; ai.kickTimer = 20; }
                else { ai.grabbing = true; ai.grabTimer = 10; }
            }
            else
            {
                if (playerToRight) ai.x += 200 * deltaTime;
                else ai.x -= 200 * deltaTime;
            }
        }

        if (!ai.jumping && rand() % 300 == 0)
        {
            ai.vy = -600.0f;
            ai.jumping = true;
        }

        if (ai.x < 0) ai.x = 0;
        if (ai.x > 750) ai.x = 750;
    }
};

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

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

    SDL_AudioSpec themeSpec, hitSpec;
    Uint8 *themeData, *hitData;
    Uint32 themeLen, hitLen;

    SDL_LoadWAV("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\theme.wav", &themeSpec, &themeData, &themeLen);
    SDL_LoadWAV("C:\\Users\\DC\\OneDrive\\Desktop\\fighter-game\\hit.wav", &hitSpec, &hitData, &hitLen);

    SDL_AudioStream *themeStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &themeSpec, NULL, NULL);
    SDL_AudioStream *hitStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &hitSpec, NULL, NULL);

    auto playTheme = [&]() {
        SDL_ClearAudioStream(themeStream);
        for (int i = 0; i < 10; i++)
            SDL_PutAudioStreamData(themeStream, themeData, themeLen);
        SDL_ResumeAudioStreamDevice(themeStream);
    };

    auto playHit = [&]() {
        SDL_ClearAudioStream(hitStream);
        SDL_PutAudioStreamData(hitStream, hitData, hitLen);
        SDL_ResumeAudioStreamDevice(hitStream);
    };

    auto stopTheme = [&]() {
        SDL_ClearAudioStream(themeStream);
        SDL_PauseAudioStreamDevice(themeStream);
    };

    Player p1("Player 1", p1tex, 100);
    Player p2("Computer", p2tex, 650);
    AI ai;
    bool vsComputer = false;

    Projectile p1proj, p2proj;

    float gravity = 1500.0f;
    float jumpForce = -600.0f;
    float groundY = 400.0f;
    int currentRound = 1;
    int p1wins = 0, p2wins = 0;
    std::string winner = "";
    GameState state = MENU;
    float stateTimer = 1.5f;
    float roundTimer = 60.0f;

    float flashTimer = 0.0f;
    float flashMaxTime = 0.5f;
    bool flashRed = false;
    float slowTimer = 0.0f;
    float slowMaxTime = 1.0f;
    bool slowActive = false;

    std::string p1NameInput = "";
    std::string p2NameInput = "";
    bool enteringP1 = true;

    SDL_FRect btn2P = {250, 250, 300, 70};
    SDL_FRect btnVSComp = {250, 350, 300, 70};

    auto drawButton = [&](SDL_FRect rect, const char *text, bool hovered) {
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
            (float)surf->h
        };
        SDL_RenderTexture(renderer, tex, NULL, &textRect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto drawText = [&](TTF_Font *font, const char *text, SDL_Color color, float x, float y) {
        if (std::string(text).empty()) return;
        SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect rect = {x, y, (float)surf->w, (float)surf->h};
        SDL_RenderTexture(renderer, tex, NULL, &rect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto drawTextCentered = [&](TTF_Font *font, const char *text, SDL_Color color, float y) {
        if (std::string(text).empty()) return;
        SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FRect rect = {(800 - (float)surf->w) / 2, y, (float)surf->w, (float)surf->h};
        SDL_RenderTexture(renderer, tex, NULL, &rect);
        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
    };

    auto resetGame = [&]() {
        p1.reset(100);
        p2.reset(650);
        ai.reset();
        p1proj.active = false;
        p2proj.active = false;
        currentRound = 1;
        p1wins = 0;
        p2wins = 0;
        winner = "";
        stateTimer = 1.5f;
        roundTimer = 60.0f;
        flashTimer = 0.0f;
        flashRed = false;
        slowTimer = 0.0f;
        slowActive = false;
    };

    auto resetRound = [&]() {
        p1.reset(100);
        p2.reset(650);
        ai.reset();
        p1proj.active = false;
        p2proj.active = false;
        stateTimer = 1.5f;
        roundTimer = 60.0f;
        flashTimer = 0.0f;
        flashRed = false;
        slowTimer = 0.0f;
        slowActive = false;
    };

    auto triggerKOEffects = [&]() {
        flashTimer = flashMaxTime;
        flashRed = false;
        slowTimer = slowMaxTime;
        slowActive = true;
    };

    auto triggerGrabFlash = [&]() {
        flashTimer = flashMaxTime;
        flashRed = true;
    };

    Uint64 lastTime = SDL_GetTicks();
    bool running = true;
    SDL_Event event;

    while (running)
    {
        Uint64 currentTime = SDL_GetTicks();
        float rawDeltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        float deltaTime = slowActive ? rawDeltaTime * 0.2f : rawDeltaTime;

        if (slowActive)
        {
            slowTimer -= rawDeltaTime;
            if (slowTimer <= 0) { slowTimer = 0; slowActive = false; }
        }

        if (flashTimer > 0)
            flashTimer -= rawDeltaTime;

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
                if (state == MENU)
                {
                    if (hover2P)
                    {
                        vsComputer = false;
                        p1NameInput = "";
                        p2NameInput = "";
                        enteringP1 = true;
                        state = ENTER_NAMES;
                        SDL_StartTextInput(window);
                    }
                    if (hoverVSComp)
                    {
                        vsComputer = true;
                        p1NameInput = "";
                        enteringP1 = true;
                        state = ENTER_NAMES;
                        SDL_StartTextInput(window);
                    }
                }
            }

            if (event.type == SDL_EVENT_TEXT_INPUT && state == ENTER_NAMES)
            {
                if (enteringP1)
                {
                    if (p1NameInput.length() < 12)
                        p1NameInput += event.text.text;
                }
                else
                {
                    if (p2NameInput.length() < 12)
                        p2NameInput += event.text.text;
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (state == MENU && event.key.scancode == SDL_SCANCODE_C)
                    state = CONTROLS;
                if (state == CONTROLS && event.key.scancode == SDL_SCANCODE_ESCAPE)
                    state = MENU;

                if (state == ENTER_NAMES)
                {
                    if (event.key.scancode == SDL_SCANCODE_BACKSPACE)
                    {
                        if (enteringP1 && !p1NameInput.empty())
                            p1NameInput.pop_back();
                        else if (!enteringP1 && !p2NameInput.empty())
                            p2NameInput.pop_back();
                    }
                    if (event.key.scancode == SDL_SCANCODE_RETURN)
                    {
                        if (enteringP1)
                        {
                            if (p1NameInput.empty()) p1NameInput = "Player 1";
                            if (vsComputer)
                            {
                                p1.name = p1NameInput;
                                p2.name = "Computer";
                                SDL_StopTextInput(window);
                                resetGame();
                                state = SHOW_ROUND;
                                playTheme();
                            }
                            else enteringP1 = false;
                        }
                        else
                        {
                            if (p2NameInput.empty()) p2NameInput = "Player 2";
                            p1.name = p1NameInput;
                            p2.name = p2NameInput;
                            SDL_StopTextInput(window);
                            resetGame();
                            state = SHOW_ROUND;
                            playTheme();
                        }
                    }
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        SDL_StopTextInput(window);
                        state = MENU;
                    }
                }

                if (state == PLAYING && event.key.scancode == SDL_SCANCODE_ESCAPE)
                    state = PAUSED;
                else if (state == PAUSED && event.key.scancode == SDL_SCANCODE_ESCAPE)
                    state = PLAYING;
                else if (state == PAUSED && event.key.scancode == SDL_SCANCODE_M)
                {
                    resetGame();
                    state = MENU;
                    stopTheme();
                }

                if (state == GAME_OVER && event.key.scancode == SDL_SCANCODE_SPACE)
                {
                    resetGame();
                    state = MENU;
                    stopTheme();
                }

                if (state == PLAYING)
                {
                    if (event.key.scancode == SDL_SCANCODE_W && !p1.taunting && !p1.charging)
                    {
                        if (!p1.jumping) { p1.vy = jumpForce; p1.jumping = true; }
                        else if (!p1.hasDoubleJumped) { p1.vy = jumpForce; p1.hasDoubleJumped = true; }
                    }
                    if (!vsComputer && event.key.scancode == SDL_SCANCODE_UP && !p2.taunting && !p2.charging)
                    {
                        if (!p2.jumping) { p2.vy = jumpForce; p2.jumping = true; }
                        else if (!p2.hasDoubleJumped) { p2.vy = jumpForce; p2.hasDoubleJumped = true; }
                    }
                    if (event.key.scancode == SDL_SCANCODE_F && !p1.blocking && !p1.taunting && !p1.grabbing && !p1.charging)
                    {
                        if (!p1.jumping && !p1.attacking && !p1.kicking) { p1.attacking = true; p1.attackTimer = 15; }
                        else if (p1.jumping && !p1.airAttacking) { p1.airAttacking = true; p1.airAttackTimer = 15; }
                    }
                    if (!vsComputer && event.key.scancode == SDL_SCANCODE_K && !p2.blocking && !p2.taunting && !p2.grabbing && !p2.charging)
                    {
                        if (!p2.jumping && !p2.attacking && !p2.kicking) { p2.attacking = true; p2.attackTimer = 15; }
                        else if (p2.jumping && !p2.airAttacking) { p2.airAttacking = true; p2.airAttackTimer = 15; }
                    }
                    if (event.key.scancode == SDL_SCANCODE_G && !p1.kicking && !p1.blocking && !p1.attacking && !p1.taunting && !p1.grabbing && !p1.jumping && !p1.charging)
                    {
                        p1.kicking = true;
                        p1.kickTimer = 20;
                    }
                    if (!vsComputer && event.key.scancode == SDL_SCANCODE_L && !p2.kicking && !p2.blocking && !p2.attacking && !p2.taunting && !p2.grabbing && !p2.jumping && !p2.charging)
                    {
                        p2.kicking = true;
                        p2.kickTimer = 20;
                    }
                    if (event.key.scancode == SDL_SCANCODE_H && !p1.grabbing && !p1.attacking && !p1.kicking && !p1.blocking && !p1.taunting && !p1.jumping && !p1.charging)
                    {
                        p1.grabbing = true;
                        p1.grabTimer = 10;
                    }
                    if (!vsComputer && event.key.scancode == SDL_SCANCODE_O && !p2.grabbing && !p2.attacking && !p2.kicking && !p2.blocking && !p2.taunting && !p2.jumping && !p2.charging)
                    {
                        p2.grabbing = true;
                        p2.grabTimer = 10;
                    }
                    if (event.key.scancode == SDL_SCANCODE_E && p1.isFullyCharged() && !p1proj.active)
                    {
                        p1proj.fire(p1.x + 80, p1.y + 50, true, true);
                        p1.charging = false;
                        p1.chargeTimer = 0;
                    }
                    if (!vsComputer && event.key.scancode == SDL_SCANCODE_I && p2.isFullyCharged() && !p2proj.active)
                    {
                        p2proj.fire(p2.x - 20, p2.y + 50, false, false);
                        p2.charging = false;
                        p2.chargeTimer = 0;
                    }
                }
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                if (event.key.scancode == SDL_SCANCODE_E && !p1.isFullyCharged())
                {
                    p1.charging = false;
                    p1.chargeTimer = 0;
                }
                if (!vsComputer && event.key.scancode == SDL_SCANCODE_I && !p2.isFullyCharged())
                {
                    p2.charging = false;
                    p2.chargeTimer = 0;
                }
            }
        }

        if (state == PLAYING)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);
            p1.blocking = keys[SDL_SCANCODE_S] && !p1.taunting && !p1.jumping && !p1.charging;
            p1.taunting = keys[SDL_SCANCODE_T] && !p1.attacking && !p1.kicking && !p1.blocking && !p1.grabbing && !p1.jumping && !p1.charging;

            if (keys[SDL_SCANCODE_E] && !p1.attacking && !p1.kicking && !p1.blocking && !p1.taunting && !p1.grabbing && !p1.jumping && !p1proj.active)
            {
                p1.charging = true;
                if (p1.chargeTimer < p1.chargeMaxTime)
                    p1.chargeTimer += deltaTime;
            }

            if (!vsComputer)
            {
                p2.blocking = keys[SDL_SCANCODE_DOWN] && !p2.taunting && !p2.jumping && !p2.charging;
                p2.taunting = keys[SDL_SCANCODE_P] && !p2.attacking && !p2.kicking && !p2.blocking && !p2.grabbing && !p2.jumping && !p2.charging;

                if (keys[SDL_SCANCODE_I] && !p2.attacking && !p2.kicking && !p2.blocking && !p2.taunting && !p2.grabbing && !p2.jumping && !p2proj.active)
                {
                    p2.charging = true;
                    if (p2.chargeTimer < p2.chargeMaxTime)
                        p2.chargeTimer += deltaTime;
                }
            }
        }

        if (state == SHOW_ROUND)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0) { state = SHOW_FIGHT; stateTimer = 1.0f; }
        }
        else if (state == SHOW_FIGHT)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0) state = PLAYING;
        }
        else if (state == PAUSED) { }
        else if (state == PLAYING)
        {
            const bool *keys = SDL_GetKeyboardState(NULL);

            if (!p1.blocking && !p1.taunting && !p1.charging)
            {
                if (keys[SDL_SCANCODE_A]) p1.x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_D]) p1.x += 300 * deltaTime;
            }
            if (!vsComputer && !p2.blocking && !p2.taunting && !p2.charging)
            {
                if (keys[SDL_SCANCODE_LEFT]) p2.x -= 300 * deltaTime;
                if (keys[SDL_SCANCODE_RIGHT]) p2.x += 300 * deltaTime;
            }

            if (p1.x < 0) p1.x = 0;
            if (p1.x > 750) p1.x = 750;
            if (p2.x < 0) p2.x = 0;
            if (p2.x > 750) p2.x = 750;

            if (vsComputer) ai.update(p2, p1, deltaTime);

            p1.update(deltaTime, gravity, groundY);
            p2.update(deltaTime, gravity, groundY);
            p1proj.update(deltaTime);
            p2proj.update(deltaTime);

            if (p1.x < 0) p1.x = 0;
            if (p1.x > 750) p1.x = 750;
            if (p2.x < 0) p2.x = 0;
            if (p2.x > 750) p2.x = 750;

            roundTimer -= deltaTime;
            if (roundTimer <= 0)
            {
                roundTimer = 0;
                if (p1.health > p2.health) p1wins++;
                else if (p2.health > p1.health) p2wins++;

                if (p1wins == 2) { winner = p1.name + " Wins!"; state = GAME_OVER; stopTheme(); }
                else if (p2wins == 2) { winner = p2.name + " Wins!"; state = GAME_OVER; stopTheme(); }
                else { currentRound++; state = ROUND_OVER; stateTimer = 2.0f; }
            }

            SDL_FRect p1rect = p1.getRect();
            SDL_FRect p2rect = p2.getRect();
            SDL_FRect p1punch = p1.getPunchHitbox(true);
            SDL_FRect p2punch = p2.getPunchHitbox(false);
            SDL_FRect p1kick = p1.getKickHitbox(true);
            SDL_FRect p2kick = p2.getKickHitbox(false);
            SDL_FRect p1grab = p1.getGrabHitbox(true);
            SDL_FRect p2grab = p2.getGrabHitbox(false);
            SDL_FRect p1air = p1.getAirAttackHitbox(true);
            SDL_FRect p2air = p2.getAirAttackHitbox(false);

            if (p1.attacking && SDL_HasRectIntersectionFloat(&p1punch, &p2rect))
            {
                if (p2.blocking) { p2.health -= 0.1f; if (p2.health < 0) p2.health = 0; }
                else { p2.takeDamage(0.5f); playHit(); }
            }
            if (p2.attacking && SDL_HasRectIntersectionFloat(&p2punch, &p1rect))
            {
                if (p1.blocking) { p1.health -= 0.1f; if (p1.health < 0) p1.health = 0; }
                else { p1.takeDamage(0.5f); playHit(); }
            }
            if (p1.kicking && SDL_HasRectIntersectionFloat(&p1kick, &p2rect))
            {
                if (p2.blocking) { p2.health -= 0.15f; if (p2.health < 0) p2.health = 0; }
                else { p2.takeDamage(0.7f); playHit(); }
            }
            if (p2.kicking && SDL_HasRectIntersectionFloat(&p2kick, &p1rect))
            {
                if (p1.blocking) { p1.health -= 0.15f; if (p1.health < 0) p1.health = 0; }
                else { p1.takeDamage(0.7f); playHit(); }
            }
            if (p1.grabbing && SDL_HasRectIntersectionFloat(&p1grab, &p2rect))
            {
                if (p2.blocking) { p1.grabbing = false; }
                else { p2.takeDamage(0.3f); p2.getThrown(true); p1.grabbing = false; triggerGrabFlash(); playHit(); }
            }
            if (p2.grabbing && SDL_HasRectIntersectionFloat(&p2grab, &p1rect))
            {
                if (p1.blocking) { p2.grabbing = false; }
                else { p1.takeDamage(0.3f); p1.getThrown(false); p2.grabbing = false; triggerGrabFlash(); playHit(); }
            }
            if (p1.airAttacking && SDL_HasRectIntersectionFloat(&p1air, &p2rect))
            {
                if (p2.blocking) { p2.health -= 0.1f; if (p2.health < 0) p2.health = 0; }
                else { p2.takeDamage(0.5f); playHit(); }
            }
            if (p2.airAttacking && SDL_HasRectIntersectionFloat(&p2air, &p1rect))
            {
                if (p1.blocking) { p1.health -= 0.1f; if (p1.health < 0) p1.health = 0; }
                else { p1.takeDamage(0.5f); playHit(); }
            }

            // Projectile hits
            if (p1proj.active)
            {
                SDL_FRect projRect = p1proj.getRect();
                if (SDL_HasRectIntersectionFloat(&projRect, &p2rect))
                {
                    if (p2.blocking) { p2.health -= 0.3f; if (p2.health < 0) p2.health = 0; }
                    else { p2.takeDamage(2.5f); playHit(); }
                    p1proj.active = false;
                }
            }
            if (p2proj.active)
            {
                SDL_FRect projRect = p2proj.getRect();
                if (SDL_HasRectIntersectionFloat(&projRect, &p1rect))
                {
                    if (p1.blocking) { p1.health -= 0.3f; if (p1.health < 0) p1.health = 0; }
                    else { p1.takeDamage(2.5f); playHit(); }
                    p2proj.active = false;
                }
            }

            // Projectiles cancel each other
            if (p1proj.active && p2proj.active)
            {
                SDL_FRect r1 = p1proj.getRect();
                SDL_FRect r2 = p2proj.getRect();
                if (SDL_HasRectIntersectionFloat(&r1, &r2))
                {
                    p1proj.active = false;
                    p2proj.active = false;
                    flashTimer = flashMaxTime;
                    flashRed = false;
                }
            }

            if (p1.health <= 0 || p2.health <= 0)
            {
                if (!slowActive)
                {
                    triggerKOEffects();
                    if (p1.health <= 0) p2wins++;
                    if (p2.health <= 0) p1wins++;

                    if (p1wins == 2) { winner = p1.name + " Wins!"; state = GAME_OVER; stopTheme(); }
                    else if (p2wins == 2) { winner = p2.name + " Wins!"; state = GAME_OVER; stopTheme(); }
                    else { currentRound++; state = ROUND_OVER; stateTimer = 2.0f; }
                }
            }
        }
        else if (state == ROUND_OVER)
        {
            stateTimer -= deltaTime;
            if (stateTimer <= 0) { resetRound(); state = SHOW_ROUND; }
        }

        // DRAW
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold = {255, 215, 0, 255};
        SDL_Color red = {255, 0, 0, 255};
        SDL_Color black = {0, 0, 0, 255};
        SDL_Color yellow = {255, 215, 0, 255};
        SDL_Color gray = {180, 180, 180, 255};

        if (state == MENU)
        {
            drawTextCentered(fontTitle, "KING OF GOON", gold, 100);
            drawButton(btn2P, "2 Player", hover2P);
            drawButton(btnVSComp, "VS Computer", hoverVSComp);
            drawTextCentered(fontSmall, "Press C for controls", gray, 480);
        }
        else if (state == CONTROLS)
        {
            drawTextCentered(fontBig, "CONTROLS", gold, 30);
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
            SDL_RenderLine(renderer, 400, 120, 400, 560);

            drawText(fontMed, "Player 1", white, 80, 120);
            drawText(fontSmall, "Move         A / D", gray, 60, 175);
            drawText(fontSmall, "Jump         W", gray, 60, 210);
            drawText(fontSmall, "Double Jump  W (air)", gray, 60, 245);
            drawText(fontSmall, "Punch        F", gray, 60, 280);
            drawText(fontSmall, "Air Attack   F (air)", gray, 60, 315);
            drawText(fontSmall, "Kick         G", gray, 60, 350);
            drawText(fontSmall, "Block        S", gray, 60, 385);
            drawText(fontSmall, "Taunt        T", gray, 60, 420);
            drawText(fontSmall, "Grab         H", gray, 60, 455);
            drawText(fontSmall, "Special      Hold E", gray, 60, 490);

            drawText(fontMed, "Player 2", white, 480, 120);
            drawText(fontSmall, "Move         LEFT / RIGHT", gray, 430, 175);
            drawText(fontSmall, "Jump         UP", gray, 430, 210);
            drawText(fontSmall, "Double Jump  UP (air)", gray, 430, 245);
            drawText(fontSmall, "Punch        K", gray, 430, 280);
            drawText(fontSmall, "Air Attack   K (air)", gray, 430, 315);
            drawText(fontSmall, "Kick         L", gray, 430, 350);
            drawText(fontSmall, "Block        DOWN", gray, 430, 385);
            drawText(fontSmall, "Taunt        P", gray, 430, 420);
            drawText(fontSmall, "Grab         O", gray, 430, 455);
            drawText(fontSmall, "Special      Hold I", gray, 430, 490);

            drawTextCentered(fontSmall, "Press ESC to go back", gray, 540);
        }
        else if (state == ENTER_NAMES)
        {
            drawTextCentered(fontBig, "KING OF GOON", gold, 80);
            if (enteringP1)
            {
                drawTextCentered(fontMed, "Enter Your Name:", white, 230);
                std::string display = p1NameInput + "|";
                drawTextCentered(fontBig, display.c_str(), gold, 310);
                drawTextCentered(fontSmall, "Press ENTER to confirm", gray, 430);
            }
            else
            {
                drawTextCentered(fontSmall, ("P1: " + p1NameInput).c_str(), gold, 200);
                drawTextCentered(fontMed, "Enter Player 2 Name:", white, 260);
                std::string display = p2NameInput + "|";
                drawTextCentered(fontBig, display.c_str(), gold, 310);
                drawTextCentered(fontSmall, "Press ENTER to confirm", gray, 430);
            }
            drawTextCentered(fontSmall, "Press ESC to go back", gray, 470);
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

            SDL_FRect p1displayBar = {50, 50, (p1.displayHealth / 100.0f) * 200, 20};
            SDL_FRect p2displayBar = {550, 50, (p2.displayHealth / 100.0f) * 200, 20};
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            SDL_RenderFillRect(renderer, &p1displayBar);
            SDL_RenderFillRect(renderer, &p2displayBar);

            SDL_FRect p1healthBar = {50, 50, (p1.health / 100.0f) * 200, 20};
            SDL_FRect p2healthBar = {550, 50, (p2.health / 100.0f) * 200, 20};
            SDL_SetRenderDrawColor(renderer, p1.health < 30 ? 255 : 0, p1.health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p1healthBar);
            SDL_SetRenderDrawColor(renderer, p2.health < 30 ? 255 : 0, p2.health >= 30 ? 255 : 0, 0, 255);
            SDL_RenderFillRect(renderer, &p2healthBar);

            drawText(fontSmall, p1.name.c_str(), white, 50, 5);
            drawText(fontSmall, p2.name.c_str(), white, 550, 5);

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

            // Round counter
            std::string roundText = "Round " + std::to_string(currentRound) + "/3";
            drawTextCentered(fontSmall, roundText.c_str(), gray, 75);

            // Projectiles
            if (p1proj.active)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_FRect projRect = p1proj.getRect();
                SDL_RenderFillRect(renderer, &projRect);
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 100);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_FRect glowRect = {projRect.x - 5, projRect.y - 5, projRect.w + 10, projRect.h + 10};
                SDL_RenderFillRect(renderer, &glowRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            if (p2proj.active)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_FRect projRect = p2proj.getRect();
                SDL_RenderFillRect(renderer, &projRect);
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 100);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_FRect glowRect = {projRect.x - 5, projRect.y - 5, projRect.w + 10, projRect.h + 10};
                SDL_RenderFillRect(renderer, &glowRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

            p1.draw(renderer);
            p2.draw(renderer);

            if (flashTimer > 0)
            {
                Uint8 alpha = (Uint8)((flashTimer / flashMaxTime) * 200);
                if (flashRed)
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
                else
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_FRect flashRect = {0, 0, 800, 600};
                SDL_RenderFillRect(renderer, &flashRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

            if (state == SHOW_ROUND)
            {
                std::string rt = "Round " + std::to_string(currentRound);
                drawTextCentered(fontBig, rt.c_str(), white, (600 - 80) / 2);
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
            else if (state == PAUSED)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_FRect overlay = {0, 0, 800, 600};
                SDL_RenderFillRect(renderer, &overlay);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

                drawTextCentered(fontBig, "PAUSED", white, 220);
                drawTextCentered(fontMed, "Press ESC to resume", white, 320);
                drawTextCentered(fontMed, "Press M for menu", white, 380);
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