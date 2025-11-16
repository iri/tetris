#include "tetris.h"
#include <string.h>

/**
 * @brief Main program
 *
 * @param argc : Number of arguments passed to the program
 * @param argv : Values of arguments passed to the program
 * @return int
 */
int main(int argc, char **argv)
{
    /* Initializes the timer, audio, video, joystick,
    haptic, gamecontroller and events subsystems */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    // Acquire display parameters
    int nDispModes = SDL_GetNumDisplayModes(0);

    SDL_DisplayMode DM;

    for (int i = 0; i < nDispModes; i++)
    {
        SDL_GetDisplayMode(0, i, &DM);
        int w = DM.w;
        int h = DM.h;
        int rr = DM.refresh_rate;
        printf("==   %d         %d   %d   %d\n", i, w, h, rr);
    }


    SDL_GetCurrentDisplayMode(0, &DM);
    printf("==  current    %d   %d   %d\n", DM.w, DM.h, DM.refresh_rate);

    tConfig CONF;
    memset(&CONF, 0, sizeof(CONF));
    CONF.w = 1200;
    CONF.h = 800;
    CONF.x = (DM.w - CONF.w) / 2;
    CONF.y = (DM.h - CONF.h) / 2;
    printf("%d   %d    %d    %d\n",CONF.x,CONF.y,CONF.w,CONF.h);

    tState ST = {.colors = {{0, 0, 0},      // transparent
                            {228, 26, 28},  // red
                            {255, 255, 51}, // yellow
                            {255, 127, 0},  // orange
                            {77, 175, 74},  // green
                            {152, 78, 163}, // violet
                            {80, 80, 80}},  // gray
                 .items = {{0, 1, 0, 0,     //
                            0, 1, 0, 0,     //
                            0, 1, 0, 0,     //
                            0, 1, 0, 0},    // I
                           {0, 0, 0, 0,     //
                            0, 2, 2, 0,     //
                            0, 2, 2, 0,     //
                            0, 0, 0, 0},    // O
                           {0, 0, 0, 0,     //
                            0, 3, 0, 0,     //
                            0, 3, 0, 0,     //
                            0, 3, 3, 0},    // L
                           {0, 0, 0, 0,     //
                            0, 0, 3, 0,     //
                            0, 0, 3, 0,     //
                            0, 3, 3, 0},    // J
                           {0, 0, 0, 0,     //
                            0, 4, 0, 0,     //
                            0, 4, 4, 0,     //
                            0, 0, 4, 0},    // S
                           {0, 0, 0, 0,     //
                            0, 0, 4, 0,     //
                            0, 4, 4, 0,     //
                            0, 4, 0, 0},    // 4
                           {0, 0, 0, 0,     //
                            0, 5, 0, 0,     //
                            5, 5, 5, 0,     //
                            0, 0, 0, 0}},   // T
                 .fps = 60,
                 .block_size = 25,
                 .GAME_STATE = GAME_WELCOME,
                 .TIMER_FPS = {0, 1000 / ST.fps},
                 .TIMER_1 = {0, 1000},
                 .TIMER_2 = {0, 75},
                 .ITEM_ID = -1};

    /* Create a window */
    uint32_t flags = 0;
    SDL_Window *wind = SDL_CreateWindow("TETRIS", CONF.x, CONF.y, CONF.w, CONF.h, flags);
    if (!wind)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    /* Create a renderer */
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, render_flags);
    if (!rend)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(wind);
        SDL_Quit();
        return 0;
    }
    /* Main loop */
    bool running = true;
    bool put_pressed = false, left_pressed = false, right_pressed = false, up_pressed = false;
    bool put_processed = false, left_processed = false, right_processed = false, up_processed = false;
    SDL_Rect rectScr = {0, 0, CONF.w, CONF.h};
    SDL_Event event;

    ST.glass_w = GLASS_W * ST.block_size;
    ST.glass_h = GLASS_H * ST.block_size;
    ST.glass_x = (CONF.w - ST.glass_w) / 2;
    ST.glass_y = (CONF.h - ST.glass_h) / 2;

    timer_start(&ST.TIMER_FPS);
    timer_start(&ST.TIMER_1);
    timer_start(&ST.TIMER_2);

    while (running)
    {
        /* Process events */
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_Q:
                    running = false;
                    break;
                case SDL_SCANCODE_SPACE:
                    if (ST.GAME_STATE == GAME_WELCOME) {
                        ST.GAME_STATE = GAME_STARTED;
                    }
                    put_pressed = true;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = true;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = true;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up_pressed = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_SPACE:
                    put_pressed = false;
                    put_processed = false;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = false;
                    left_processed = false;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = false;
                    right_processed = false;
                    break;
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up_pressed = false;
                    up_processed = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        // Processing
        // TIMER_FPS handling (1000/60 ms)
        if (is_timer_tick(&ST.TIMER_FPS))
        {
            // Process slow timers
            // TIMER_1 handling (1000 ms)
            if (is_timer_tick(&ST.TIMER_1))
            {
                printf("%s\n", getGameState(ST.GAME_STATE));
                switch (ST.GAME_STATE)
                {
                case GAME_WELCOME:
                    break;
                case GAME_STARTED:
                    break;
                case ITEM_STARTED:
                    break;
                case ITEM_FALLING:
                    fallStep(&ST);
                    break;
                case ITEM_FALLING_FAST:
                    break;
                case ITEM_STOPPED:
                    break;
                case GAME_FINISHED:
                    break;
                }

                timer_start(&ST.TIMER_1);
            }

            // TIMER_2 handling (100 ms)
            if (is_timer_tick(&ST.TIMER_2))
            {
                printf("%s\n", getGameState(ST.GAME_STATE));
                switch (ST.GAME_STATE)
                {
                case GAME_WELCOME:
                    break;
                case GAME_STARTED:
                    break;
                case ITEM_STARTED:
                    break;
                case ITEM_FALLING:
                    break;
                case ITEM_FALLING_FAST:
                    fallStep(&ST);
                    break;
                case ITEM_STOPPED:
                    if (!checkRemoveFullLine(&ST))
                    {
                        ST.GAME_STATE = ITEM_STARTED;
                    }
                    break;
                case GAME_FINISHED:
                    break;
                }

                timer_start(&ST.TIMER_2);
            }

            // Process FPS timer
            /* Clear screen */
            SDL_SetRenderDrawColor(rend, ST.colors[6][0], ST.colors[6][1], ST.colors[6][2], 255);
            SDL_RenderFillRect(rend, &rectScr);

            /* Draw glass */
            drawGlass(rend, &ST);

            switch (ST.GAME_STATE)
            {
            case GAME_WELCOME:
                drawWelcomeScreen(rend, &ST);
                break;
            case GAME_STARTED:
                clearGlass(&ST);
                ST.GAME_STATE = ITEM_STARTED;
                break;
            case ITEM_STARTED:
                ST.ITEM_ID = rand() % 7;
                ST.x = (CONF.w - ITEMBLOCKS * ST.block_size) / 2;
                ST.y = ITEMBLOCKS * ST.block_size / 2;
                // init item margins
                getLeftMargins(&ST);
                getRightMargins(&ST);
                getBottomMargins(&ST);
                // update state
                updState(&ST);
                drawItem(rend, ST.x, ST.y, &ST);
                SDL_Delay(500);
                ST.GAME_STATE = ITEM_FALLING;
                break;
            case ITEM_FALLING:
            case ITEM_FALLING_FAST:
                if (left_pressed && !left_processed)
                {
                    if (!checkItemLeft(&ST))
                    {
                        ST.x -= ST.block_size;
                        updState(&ST);
                    }
                    left_processed = true;
                }
                if (right_pressed && !right_processed)
                {
                    if (!checkItemRight(&ST))
                    {
                        ST.x += ST.block_size;
                        updState(&ST);
                    }
                    right_processed = true;
                }
                if (up_pressed && !up_processed)
                {
                    rotateItem(&ST);
                    updState(&ST);
                    up_processed = true;
                }
                if (put_pressed && !put_processed)
                {
                    ST.GAME_STATE = ITEM_FALLING_FAST;
                    put_processed = true;
                }
                drawItem(rend, ST.x, ST.y, &ST);
                break;
            case ITEM_STOPPED:
                drawItem(rend, ST.x, ST.y, &ST);
                break;
            case GAME_FINISHED:
                break;
            }

            /* Draw to window and loop */
            SDL_RenderPresent(rend);

            timer_start(&ST.TIMER_FPS);
        }
        else
        {
            SDL_Delay(1);
        }
    }

    /* Release resources */
    TTF_Quit();
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
}
