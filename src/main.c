#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct _tconfig
{
    int _l, _r, _t, _b, _f;
    int w, h;
    int x, y;
} tConfig;

bool parse_opt_arg_uint(int argc, char **argv, int *ind, int *res)
{
    int rc = true;
    int r;

    if (argv[*ind][0] != '-') // non-negative int only
    {
        if (strcmp("0", argv[*ind]) == 0)
        {
            *res = 0;
            *ind += 1;
        }
        else
        {
            r = atoi(argv[*ind]);
            if (r != 0)
            {
                *res = r;
                *ind += 1;
            }
            else
            {
                rc = false;
            }
        }
    }
    else
    {
        rc = false;
    }
    return rc;
}

bool parse_opt_arg_str(int argc, char **argv, int *ind, char *res, int len)
{
    int rc = true;

    if (argv[*ind][0] != '-') // must not start with "-"
    {
        strncpy(res, argv[*ind], len);
        *ind += 1;
    }
    else
    {
        *res = 0;
    }
    return rc;
}

void parse_args(int argc, char **argv, tConfig *conf)
{
    int option;
    extern char *optarg;
    extern int optind;
    char s[10];

    memset(conf, 0, sizeof(*conf));
    memset(s, 0, sizeof(s));

    while ((option = getopt(argc, argv, "hfirxd")) != -1)
    {
        printf("%d   %s     %c\n", optind, argv[optind - 1], option);
        switch (option)
        {
        case 'd':
            if (parse_opt_arg_uint(argc, argv, &optind, &conf->w) &&
                parse_opt_arg_uint(argc, argv, &optind, &conf->h) && parse_opt_arg_str(argc, argv, &optind, s, 10))
            {
                printf("option args : %i   %i   %s\n", conf->w, conf->h, s);
                if (strchr(s, 'f'))
                {
                    conf->_f = 1;
                }
                else
                {
                    if (strchr(s, 'l'))
                    {
                        conf->_l = 1;
                    }
                    if (strchr(s, 'r'))
                    {
                        conf->_r = 1;
                    }
                    if (strchr(s, 't'))
                    {
                        conf->_t = 1;
                    }
                    if (strchr(s, 'b'))
                    {
                        conf->_b = 1;
                    }
                }
                SDL_DisplayMode DM;
                SDL_GetCurrentDisplayMode(0, &DM);
                printf("==  current    %d   %d   %d\n", DM.w, DM.h, DM.refresh_rate);

                // fullscreen (desktop resolution)
                if (conf->_f)
                {
                    conf->x = 0;
                    conf->y = 0;
                    conf->w = DM.w;
                    conf->h = DM.h;
                }
                else
                {
                    // left-right
                    if (conf->_l + conf->_r == 2 || conf->_l + conf->_r == 0)
                    {
                        conf->x = (DM.w - conf->w) / 2;
                    }
                    else if (conf->_l)
                    {
                        conf->x = 0;
                    }
                    else if (conf->_r)
                    {
                        conf->x = DM.w - conf->w;
                    }

                    // top-bottom
                    if (conf->_t + conf->_b == 2 || conf->_t + conf->_b == 0)
                    {
                        conf->y = (DM.h - conf->h) / 2;
                    }
                    else if (conf->_t)
                    {
                        conf->y = 0;
                    }
                    else if (conf->_b)
                    {
                        conf->y = DM.h - conf->h;
                    }
                }
                printf("==  x : %d     y : %d\n", conf->x, conf->y);
            }
            else
            {
                printf("incorrect args for option -%c\n", option);
                exit(EXIT_FAILURE);
            }
            break;
        case 'c':
            printf("option : %c\n", option);
            break;
        case 'f':
            printf("filename : %s\n", optarg);
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option : %c\n", optopt);
            break;
        }
    }
}

#define SIZE 150
#define SPEED 600
#define GRAVITY 60
#define FPS 60
#define JUMP -1200

int main(int argc, char **argv)
{
    tConfig CONF;

    /* Initializes the timer, audio, video, joystick,
    haptic, gamecontroller and events subsystems */
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error initializing SDL: %s\n", SDL_GetError());
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

    // Parse command line arguments
    parse_args(argc, argv, &CONF);

    /* Create a window */
    uint32_t flags = 0;
    if (CONF._f)
    {
        flags = flags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL_Window *wind = SDL_CreateWindow("Shell0", CONF.x, CONF.y, CONF.w, CONF.h, flags);
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
    bool running = true, jump_pressed = false, can_jump = true, left_pressed = false, right_pressed = false;
    float x_pos = (CONF.w - SIZE) / 2, y_pos = (CONF.h - SIZE) / 2, x_vel = 0, y_vel = 0;
    SDL_Rect rect = {(int)x_pos, (int)y_pos, SIZE, SIZE};
    SDL_Event event;
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
                case SDL_SCANCODE_SPACE:
                    jump_pressed = true;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = true;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_SPACE:
                    jump_pressed = false;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left_pressed = false;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right_pressed = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
        /* Clear screen */
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);
        /* Move the rectangle */
        x_vel = (right_pressed - left_pressed) * SPEED;
        y_vel += GRAVITY;
        if (jump_pressed && can_jump)
        {
            can_jump = false;
            y_vel = JUMP;
        }
        x_pos += x_vel / 60;
        y_pos += y_vel / 60;
        if (x_pos <= 0)
            x_pos = 0;
        if (x_pos >= CONF.w - rect.w)
            x_pos = CONF.w - rect.w;
        if (y_pos <= 0)
            y_pos = 0;
        if (y_pos >= CONF.h - rect.h)
        {
            y_vel = 0;
            y_pos = CONF.h - rect.h;
            if (!jump_pressed)
                can_jump = true;
        }
        rect.x = (int)x_pos;
        rect.y = (int)y_pos;
        /* Draw the rectangle */
        SDL_SetRenderDrawColor(rend, 255, 200, 0, 255);
        SDL_RenderFillRect(rend, &rect);
        /* Draw to window and loop */
        SDL_RenderPresent(rend);
        SDL_Delay(1000 / FPS);
    }
    /* Release resources */
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
}
