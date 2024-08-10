#include <SDL2/SDL.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <windows.h>

#define ITEMBLOCKPIX 30
#define MAXITEMS 7
#define MAXCOLORS 7
#define ITEMBLOCKS 4

typedef uint64_t MTIMER;
void mtimer_start(MTIMER *t)
{
    *t = GetTickCount();
}
MTIMER mtimer_check(MTIMER *t)
{
    return GetTickCount() - *t;
}

typedef struct _tconfig
{
    int _l, _r, _t, _b, _f;
    int w, h;
    int x, y;
} tConfig;

typedef struct _tstate
{
    uint8_t colors[MAXCOLORS][3];
    int8_t items[MAXITEMS][ITEMBLOCKS * ITEMBLOCKS];
} tState;

void rotateMatrix(int8_t *mat)
{
    for (int8_t i = 0; i < ITEMBLOCKS / 2; i++)
    {
        for (int8_t j = i; j < ITEMBLOCKS - i - 1; j++)
        {
            // Calculate the indices in the one-dimensional array
            int8_t index1 = i * ITEMBLOCKS + j;
            int8_t index2 = j * ITEMBLOCKS + (ITEMBLOCKS - 1 - i);
            int8_t index3 = (ITEMBLOCKS - 1 - i) * ITEMBLOCKS + (ITEMBLOCKS - 1 - j);
            int8_t index4 = (ITEMBLOCKS - 1 - j) * ITEMBLOCKS + i;

            // Save the current element in a temporary variable
            int8_t temp = mat[index1];

            // Move elements from right column to top row
            mat[index1] = mat[index2];

            // Move elements from bottom row to right column
            mat[index2] = mat[index3];

            // Move elements from left column to bottom row
            mat[index3] = mat[index4];

            // Move elements from top row to left column
            mat[index4] = temp;
        }
    }
}

void printMatrix(int8_t *mat)
{
    for (int8_t i = 0; i < ITEMBLOCKS; i++)
    {
        for (int8_t j = 0; j < ITEMBLOCKS; j++)
        {
            printf("%d ", mat[i * ITEMBLOCKS + j]);
        }
        printf("\n");
    }
}

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
    memset(s, 0, sizeof(s));

    while ((option = getopt(argc, argv, "hfirxd")) != -1)
    {
        printf("%d   %s     %c\n", optind, argv[optind - 1], option);
        switch (option)
        {
        case 'd': // -d <w> <h> <{lrtbf}*>
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

void drawItem(SDL_Renderer *rend, int x, int y, tState *ST, int item)
{
    SDL_Rect rect = {x, y, ITEMBLOCKPIX, ITEMBLOCKPIX};
    int e;
    for (int8_t i = 0; i < ITEMBLOCKS; i++)
    {
        for (int8_t j = 0; j < ITEMBLOCKS; j++)
        {
            e = ST->items[item][i * ITEMBLOCKS + j];
            SDL_SetRenderDrawColor(rend, ST->colors[e][0], ST->colors[e][1], ST->colors[e][2], 255);
            rect.y = y + i * ITEMBLOCKPIX;
            rect.x = x + j * ITEMBLOCKPIX;
            SDL_RenderFillRect(rend, &rect);
        }
    }
}

int main(int argc, char **argv)
{
    tConfig CONF;
    memset(&CONF, 0, sizeof(CONF));

    tState ST = {
        .colors = {{0, 0, 0},                                        // transparent
                   {228, 26, 28},                                    // red
                   {255, 255, 51},                                   // yellow
                   {255, 127, 0},                                    // orange
                   {77, 175, 74},                                    // green
                   {152, 78, 163},                                   // violet
                   {80, 80, 80}},                                    // gray
        .items = {{0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0},  // I
                  {0, 0, 0, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0},  // O
                  {0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 3, 0},  // L
                  {0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 3, 3, 0},  // J
                  {0, 0, 0, 0, 0, 4, 0, 0, 0, 4, 4, 0, 0, 0, 4, 0},  // S
                  {0, 0, 0, 0, 0, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 0},  // 4
                  {0, 0, 0, 0, 0, 5, 0, 0, 5, 5, 5, 0, 0, 0, 0, 0}}, // T
        .fps = 60;
};

for (int i = 0; i < MAXITEMS; i++)
{
    printf("\n");
    printMatrix(ST.items[i]);
    rotateMatrix(ST.items[i]);
    printMatrix(ST.items[i]);
}

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
bool running = true;
bool put_pressed = false, left_pressed = false, right_pressed = false, up_pressed = false;
bool put_processed = false, left_processed = false, right_processed = false, up_processed = false;
SDL_Rect rectScr = {0, 0, CONF.w, CONF.h};
SDL_Event event;

int TICK_MS = 1000;

int x = 50;
int y = 50;

MTIMER t;
mtimer_start(&t);

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

    if (!up_processed)
    {
        for (int i = 0; i < MAXITEMS; i++)
        {
            rotateMatrix(ST.items[i]);
        }
        up_processed = true;
    }
    if (!left_processed)
    {
        x -= 50;
        left_processed = true;
    }
    if (!right_processed)
    {
        x += 50;
        right_processed = true;
    }
    if (!put_processed)
    {
        y += 50;
        put_processed = true;
    }

    if (mtimer_check(&t) >= TICK_MS)
    {
        printf("Tick : %I64u\n", mtimer_check(&t));
        mtimer_start(&t);
    }

    /* Clear screen */
    SDL_SetRenderDrawColor(rend, ST.colors[6][0], ST.colors[6][1], ST.colors[6][2], 255);
    SDL_RenderFillRect(rend, &rectScr);

    /* Draw scene */
    drawItem(rend, x + 50, y, &ST, 0);
    drawItem(rend, x + 200, y, &ST, 1);
    drawItem(rend, x + 350, y, &ST, 2);
    drawItem(rend, x + 500, y, &ST, 3);
    drawItem(rend, x + 650, y, &ST, 4);
    drawItem(rend, x + 800, y, &ST, 5);
    drawItem(rend, x + 950, y, &ST, 6);

    /* Draw to window and loop */
    SDL_RenderPresent(rend);
    // printf("%d  %d  %d  %d\n", left_pressed, right_pressed, up_pressed, put_pressed);
    SDL_Delay(1000 / ST.fps);
}

/* Release resources */
SDL_DestroyRenderer(rend);
SDL_DestroyWindow(wind);
SDL_Quit();
return 0;
}
